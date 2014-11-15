/*
 * main_control_task.c
 * Copyright (C) 2014  Martin Lundh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* QuadFC - A quadcopter flight controller.
 * 
 *
 * This is the main task in the QuadFC. This is the core of the QuadFC and interfaces 
 * described in main_control_task.h and main.h should always be obeyed. 

 * The task follows the structure below, initializations are done in main.c 
 * before the scheduler is started. 
 *
 * -------------------------------------------------------------------------------------------------------
 *
 *
 *      Check current flight mode.              (Landed, landing, flying, taking off?)
 *                  |
 *            Read sensors.                     (TWI communication)
 *                  |
 *        Filter sensor values.                 (kalman or complementary filer)
 *                  |
 *         Get reference signal.                (From radio control system or path planning, USART or queue)
 *                  |
 *      Calculate control signal.               (PID or LQG)
 *                  |
 *     Write control signal to motors.          (TWI communication)
 *                  |
 *          Read motor data.                    (Possibly)
 *                  |
 *       Check quadcopter status.               (Errors, self check etc.)
 *                  |
 *          Heartbeat signal.                   (Led, visual indication that everything is OK)
 *                  |
 *         Wait motor data sent                 (PDC says ok, might be moved to right before read sensors)
 *                  |
 *
 *
 *
 * ----------------------------------------------------------------------------------------------------------
 */

#include "main_control_task.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*include drivers*/
#include <gpio.h>
#include <pio.h>

/* Task includes. */
#include "led_control_task.h"
#include "communication_tasks.h"
#include "satellite_receiver_task.h"
#include "range_meter.h"
#include "state_handler.h"

#include "control_algorithm.h"
#include "imu.h"
#include "imu_signal_processing.h"
#include "motor_control.h"
#include "pwm_motor_control.h"

#include "globals.h"



void main_control_task( void *pvParameters );
void reset_control_error( control_values_pid_t *ctrl_error );
void init_twi_main( );

/*global parameters*/
control_values_pid_t *parameters_rate = NULL;
state_data_t *state = NULL;
state_data_t *setpoint = NULL;
control_signal_t *ctrl_signal = NULL;

/*
 * void create_main_control_task(void)
 *
 * Create the main task. Initialize two TWI instances and pass them to the task.
 * The main control task is time critical and therefore uses the fully asynchronous 
 * transmit functions. Work can be done while communication is in progress. 
 *
 * The data being sent must not be changed while the transmission is in progress as 
 * this might corrupt the transmitted data. If the data is allocated on the stack, the 
 * calling function should not exit before the transmission is finished, or the data
 * will get corrupted. Another way of ensuring that the data will not change is to declare
 * the variable containing the data as static or global.
 */

void create_main_control_task( void )
{
  /*Create the task*/
  portBASE_TYPE create_result;
  create_result = xTaskCreate( main_control_task,   /* The function that implements the task.  */
      (const char *const) "Main_Ctrl",       /* The name of the task. This is not used by the kernel, only aids in debugging*/
      1000,                                         /* The stack size for the task*/
      NULL,                                         /* The already configured motor data is passed as an input parameter*/
      configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

  if ( create_result != pdTRUE )
  {
    /*Error - Could not create task. TODO handle this error*/
    for ( ;; )
    {

    }
  }

}

/*
 *Task writing whatever is in the queue to the a serial port. The serial port is 
 *passed as an argument to the task. 
 */
void main_control_task( void *pvParameters )
{
  /*main_control_task execute at 500Hz*/
  static const TickType_t xPeriodTicks = (2UL / portTICK_PERIOD_MS );

  /* declared as public in main_control_task.h, variables need to be modified by the RX task to set params.*/

  parameters_rate = pvPortMalloc(
      sizeof(control_values_pid_t) ); /*PID parameters used in rate mode*/

  control_values_pid_t *ctrl_error_rate = pvPortMalloc(
      sizeof(control_values_pid_t) ); /*control error signal used in rate mode*/

  ctrl_signal = pvPortMalloc( sizeof(control_signal_t) );

  state = pvPortMalloc( sizeof(state_data_t) );     /*State information*/
  setpoint = pvPortMalloc( sizeof(state_data_t) );  /*Setpoint information*/

  receiver_data_t *local_receiver_buffer = pvPortMalloc(
      sizeof(receiver_data_t) );                                  /*Receiver information*/


  imu_data_t *imu_readings = pvPortMalloc( sizeof(imu_data_t) ); /*IMU data, parsed IMU values.*/


  int32_t motor_setpoint[MAX_MOTORS] = { 0 };

  xSemaphoreTake( x_param_mutex, portMAX_DELAY );



  /*Ensure that all mallocs returned valid pointers*/
  if (  !parameters_rate
      || !ctrl_error_rate || !state || !setpoint
      || !local_receiver_buffer || !imu_readings || !x_param_mutex
      || !ctrl_signal )
  {
    for ( ;; )
    {
      // TODO error handling!
    }
  }


  uint8_t reset_integral_error = 0; // 0 == false

  /*Data received from RC receiver task */

  local_receiver_buffer->ch0 = 0; /* throttle, should be 0 at first. */
  local_receiver_buffer->ch1 = SATELLITE_CH_CENTER;
  local_receiver_buffer->ch2 = SATELLITE_CH_CENTER;
  local_receiver_buffer->ch3 = SATELLITE_CH_CENTER;
  local_receiver_buffer->ch4 = SATELLITE_CH_CENTER;
  local_receiver_buffer->ch5 = SATELLITE_CH_CENTER;
  local_receiver_buffer->ch6 = SATELLITE_CH_CENTER;
  local_receiver_buffer->sync = 0;
  local_receiver_buffer->connection_ok = 0;

  /*Control parameter initialization for rate mode control.*/
  /*--------------------Initial parameters-------------------*/
  parameters_rate->pitch_p = 250;
  parameters_rate->pitch_i = 3;
  parameters_rate->pitch_d = 3;
  parameters_rate->roll_p = 250;
  parameters_rate->roll_i = 3;
  parameters_rate->roll_d = 3;
  parameters_rate->yaw_p = 250;
  parameters_rate->yaw_i = 3;
  parameters_rate->yaw_d = 3;
  parameters_rate->altitude_p = 1 << SHIFT_EXP;
  parameters_rate->altitude_i = 0;
  parameters_rate->altitude_d = 0;


  /*Initialization for control error in rate mode.*/
  reset_control_error( ctrl_error_rate );



  /*State initialization.*/
  state->pitch = 0;
  state->roll = 0;
  state->yaw = 0;
  state->pitch_rate = 0;
  state->roll_rate = 0;
  state->yaw_rate = 0;
  state->z_vel = 0;

  /*Setpoint initialization*/

  setpoint->pitch = 0;
  setpoint->roll = 0;
  setpoint->yaw = 0;
  setpoint->pitch_rate = 0;
  setpoint->roll_rate = 0;
  setpoint->yaw_rate = 0;
  setpoint->z_vel = 0;


  /*Rang data container*/
  int32_t ranger_data = 0;

  State_InitStateHandler();
   if(pdTRUE != State_Change(state_init))
   {
     State_Fault();
   }


  /*Initialize the IMU. A reset followed by a short delay is recommended
   * before starting the configuration.*/
  init_twi_main();
  mpu6050_initialize();

  /*Flight controller should always be started in fc_disarmed mode to prevent
   * unintentional motor arming. */
  if(pdTRUE != State_Change(state_disarmed))
  {
    State_Fault();
  }


  // TODO read from memory
  uint32_t nr_motors = 4;

  pwm_init_motor_control( nr_motors );

  TickType_t arming_counter = 0;
  uint32_t heartbeat_counter = 0;
  uint32_t spectrum_receiver_error_counter = 0;

  /*The main control loop*/

  unsigned portBASE_TYPE xLastWakeTime = xTaskGetTickCount();
  for ( ;; )
  {

    /*-----------------------------------------General info----------------------------------------------------
     *Use different control strategy depending on what flight mode is requested. Some function calls are identical
     * between the different control strategys, they are still called from within each mode due to the fact that
     * the configure mode requires that no other function uses the parameters.
     *
     */

    /*-----------------------------------------Angle mode-------------------------------------------------------
     * Angle mode uses both the accelerometer and gyro to calculate an angle estimate of the copter. The
     * setpoint is an angle compared to earth gravity and is stored in setpoint-> pitch/roll/yaw.  if the copter
     * deviates from the requested angle then the controller tries to compensate for it. If all setpoints are zero
     * the copter will try to self stabilize.*/

    /*TODO should be implemented*/

    /*-----------------------------------------Rate mode-------------------------------------------------------
     * Rate mode uses only the gyro to stabilize the copter. Only the angular rate is compensated for and the copter
     * will not self stabilize, rather this control strategy slows the dynamics of the copter down to a level where
     * a pilot can control them. The setpoint is an angular rate and is stored in setpoint->pitch_rate/roll_rate/yaw_rate.
     */
    if ( state_armed == State_GetCurrent() )
    {

      mpu6050_read_motion( imu_readings );
      get_rate_gyro( state, imu_readings );
      calc_control_signal_rate_pid( motor_setpoint, nr_motors, parameters_rate,
          ctrl_error_rate, state, setpoint, ctrl_signal );
      pwm_update_setpoint( motor_setpoint, nr_motors );
    }

    /*-----------------------------------------arming mode-------------------------------------------------------
     * When switching to fc_armed_rate_mode the fc can pass throuth an arming state to ensure that all motors
     * are armed according to the need of the motor/esc combination.*/
    else if ( state_arming == State_GetCurrent() )
    {
      if ( arming_counter >= ( 1000 / xPeriodTicks ) ) // Be in arming state for 1s.
      {
        if( !State_Change(state_armed) )
        {
          State_Fault();
        }
      }
      else
      {
        arming_counter++;
      }
    }

    /*---------------------------------------Configure mode--------------------------------------------
     * Configure mode is used to update parameters used in the control of the copter. The copter has to be
     * disarmed before entering configure mode. To enter a specific command has to be sent over USART.
     * Configure mode can be exited by either restarting the FC or
     * by sending a specific command over the USART communication channel.
     */
    else if ( state_config == State_GetCurrent() )
    {
      //Do nothing!
    }

    /*---------------------------------------Disarmed mode--------------------------------------------
     * When in disarmed mode all integral errors should be reset to avoid integral accumulation.
     *
     * It is only allowed to go into fc_configure mode if the copter is already in fc_disarmed mode.
     */

    else if ( state_disarmed == State_GetCurrent() )
    {
      if ( reset_integral_error )
      {
        reset_control_error( ctrl_error_rate );
        reset_integral_error = 0;
      }
    }

    /*------------------------------------read receiver -----------------------------------------------
     * Always read receiver.
     *TODO change from counting fc cycles to time.
     */
    if ( xQueueReceive(xQueue_receiver, local_receiver_buffer,
        mainDONT_BLOCK) == pdPASS )
    {
      /* The code in this scope will execute once every 22 ms (the frequency of new data
       * from the receiver).
       */

      /*If good signal decrease spectrum_receiver_error_counter*/
      if ( (local_receiver_buffer->connection_ok) )
      {
        translate_receiver_signal_rate( setpoint, local_receiver_buffer );
        if ( spectrum_receiver_error_counter > 0 )
        {
          spectrum_receiver_error_counter--;
        }
      }
      /*If bad connection and fc in a flightmode increase spectrum_receiver_error_counter.*/
      else if ( (State_GetCurrent() == state_armed) )
      {
        /* Allow a low number of lost frames at a time. Increase counter 4 times as fast as it is decreased.
         * This ensures that there is at least 4 good frames to each bad one.*/
        spectrum_receiver_error_counter += 4;
        /*If there has been to many errors, put the FC in disarmed mode.*/
        if ( spectrum_receiver_error_counter >= (80) )
        {
          State_Fault();
          reset_integral_error = 1;
          pwm_dissable();
          spectrum_receiver_error_counter = 0;
        }
      }
      else
      {
        //Do nothing.
      }
    }

    /*----------------------------------- Get new range (altitude) data. -------------------------------------
     *
     *
     * */
    if ( xQueueReceive(xQueue_ranger, &ranger_data, mainDONT_BLOCK) == pdPASS )
    {
      if ( (ranger_data <= 5000) && ranger_data >= 300 )
      {
        state->z_pos = ranger_data;
      }
      else
      {
        state->z_pos = 0;
      }

    }
    /*-------------------------------------State change request from receiver?----------------------------
     * Check if a fc_state change is requested, if so, change state. State change is only allowed if the copter is landed.
     */

    /*Disarm requested*/
    if ( (local_receiver_buffer->connection_ok)
        && (local_receiver_buffer->ch5 > SATELLITE_CH_CENTER)
        && (local_receiver_buffer->ch0 < 40))
    {
      if(pdTRUE == State_Change(state_disarmed))
      {
        reset_integral_error = 1;
        pwm_dissable();
      }
      else
      {
        //Not allowed to change state.
      }

    }

    /*Arming requested*/
    if ( (local_receiver_buffer->connection_ok)
        && (local_receiver_buffer->ch5 < SATELLITE_CH_CENTER)
        && (local_receiver_buffer->ch0 < 40))
    {
      if(pdTRUE == State_Change(state_arming))
      {
        arming_counter = 0;
        pwm_enable( nr_motors );
      }
      else
      {
        //Not allowed to change state.
      }
    }


    vTaskDelayUntil( &xLastWakeTime, xPeriodTicks );

    /*-------------------Heartbeat----------------------
     * Toggle an led to indicate that the FC is operational.
     */
    heartbeat_counter++;
    if ( heartbeat_counter >= 500 )
    {
      gpio_toggle_pin( PIN_31_GPIO );
      heartbeat_counter = 0;
    }
  }
  /* The task should never escape the for-loop and therefore never return.*/
}

/* Simply resets the control error (or any control_values_pid_t struct passed to it).*/
void reset_control_error( control_values_pid_t *ctrl_error )
{
  ctrl_error->pitch_p = 0;
  ctrl_error->pitch_i = 0;
  ctrl_error->pitch_d = 0;
  ctrl_error->roll_p = 0;
  ctrl_error->roll_i = 0;
  ctrl_error->roll_d = 0;
  ctrl_error->yaw_p = 0;
  ctrl_error->yaw_i = 0;
  ctrl_error->yaw_d = 0;
  ctrl_error->altitude_p = 0;
  ctrl_error->altitude_i = 0;
  ctrl_error->altitude_d = 0;
}


void init_twi_main( )
{
  freertos_peripheral_options_t async_driver_options = {
      NULL,
      0,
      (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1),
      TWI_I2C_MASTER,
      0,
  };

  freertos_peripheral_options_t async_driver_options1 = {
      NULL,
      0,
      (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1),
      TWI_I2C_MASTER,
      0,
  };

  /*Initialize twi bus 0, available at pin xx and xx on the Arduino due*/
  twi_0 = freertos_twi_master_init( TWI0,
      &async_driver_options );

  /*Initialize twi bus 1, available at pin xx and xx on the Arduino due*/
  twi_1 = freertos_twi_master_init( TWI1,
      &async_driver_options1 );

  if ( twi_0 == NULL || twi_1 == NULL )
  {
    for ( ;; )
    {
      // ERROR, TODO set error flag
    }
  }

  twi_set_speed( TWI0, 400000, sysclk_get_cpu_hz() ); /*High speed TWI setting*/
  twi_set_speed( TWI1, 400000, sysclk_get_cpu_hz() ); /*High speed TWI setting*/
}
