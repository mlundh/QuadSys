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

/* Modules */
#include "control_system.h"
#include "state_handler.h"
#include "imu.h"
#include "imu_signal_processing.h"
#include "QuadFC_MotorControl.h"
#include "parameters.h"

#include "globals.h"
#include "my_math.h"
#include "common_types.h"


void main_control_task( void *pvParameters );

// Struct only used here to pass parameters to the task.
typedef struct mainTaskParams
{
  state_data_t *state;
  state_data_t *setpoint;
  CtrlInternal_t *ctrl;
  control_signal_t *control_signal;
  MotorControlObj *motorControl;
}taskParams_t;

/*
 * void create_main_control_task(void)
 */

void create_main_control_task( void )
{
  /*Create the task*/

  taskParams_t * taskParam = pvPortMalloc(sizeof(taskParams_t));
  taskParam->ctrl = Ctrl_Create();
  taskParam->setpoint = pvPortMalloc( sizeof(state_data_t) );
  taskParam->state = pvPortMalloc( sizeof(state_data_t) );
  taskParam->control_signal = pvPortMalloc( sizeof(control_signal_t) );
  taskParam->motorControl = MotorCtrl_CreateAndInit(4);
  /*Ensure that all mallocs returned valid pointers*/
   if (!taskParam || !taskParam->ctrl || !taskParam->setpoint
       || !taskParam->state || !taskParam->control_signal
       || !taskParam->motorControl)
   {
     for ( ;; )
     {
       // TODO error handling!
     }
   }

  portBASE_TYPE create_result;
  create_result = xTaskCreate( main_control_task,   /* The function that implements the task.  */
      (const char *const) "Main_Ctrl",              /* The name of the task. This is not used by the kernel, only aids in debugging*/
      1000,                                         /* The stack size for the task*/
      taskParam,                                        /* pass params to task.*/
      configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

  if ( create_result != pdTRUE )
  {
    /*Error - Could not create task.*/
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
  static const TickType_t mainPeriodTimeMs = (2UL / portTICK_PERIOD_MS );

                       /*Receiver information*/

  taskParams_t * param = (taskParams_t*)(pvParameters);

  receiver_data_t *local_receiver_buffer = pvPortMalloc( sizeof(receiver_data_t) );
  imu_data_t *imu_readings = pvPortMalloc( sizeof(imu_data_t) );


  /*Ensure that all mallocs returned valid pointers*/
  if ( !local_receiver_buffer || !imu_readings )
  {
    for ( ;; )
    {
      // TODO error handling!
    }
  }


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


  State_InitStateHandler();


  // TODO read from memory
  uint32_t nr_motors = 4;
  int32_t motorSetpoint[nr_motors];

  TickType_t arming_counter = 0;
  uint32_t heartbeat_counter = 0;
  uint32_t spectrum_receiver_error_counter = 0;

  /*The main control loop*/

  unsigned portBASE_TYPE xLastWakeTime = xTaskGetTickCount();
  for ( ;; )
  {

   /*
    * Main state machine. Controls the state of the flight controller.
    */
    switch (State_GetCurrent())
    {
    case state_init:
      /*---------------------------------------Initialize mode------------------------
       * Initialize all modules then set the fc to dissarmed state.
       */
      mpu6050_initialize(); // Might take time, reset the last wake time to avoid errors.
      xLastWakeTime = xTaskGetTickCount();

      if(pdTRUE != State_Change(state_disarming))
      {
        State_Fault();
      }
      break;
    case state_disarmed:
      /*---------------------------------------Disarmed mode--------------------------
       * Nothing will happen in disarmed mode. Allowed to change to configure mode,
       * arming mode and fault mode.
       */
      break;
    case state_config:
      /*---------------------------------------Configure mode-------------------------
       * Configure mode is used to update parameters used in the control of the copter.
       * The copter has to be disarmed before entering configure mode. To enter a specific
       * command has to be sent over USART. Configure mode can be exited by either
       * restarting the FC or by sending a specific command over the USART communication
       * channel.
       */
      break;
    case state_arming:
      /*-----------------------------------------arming mode--------------------------
       * Before transition into armed mode the fc always passes through arming state to
       * ensure that all systems are available and ready. Use the counter method rather
       * than sleeping to allow external communication.
       */
      MotorCtrl_Enable( param->motorControl );
      Ctrl_On(param->ctrl);

      if ( arming_counter >= ( 1000 / mainPeriodTimeMs ) ) // Be in arming state for 1s.
      {
        arming_counter = 0;
        if( !State_Change(state_armed) )
        {
          State_Fault();
        }
      }
      else
      {
        arming_counter++;
      }
      break;
    case state_armed:
      /*-----------------------------------------armed mode---------------------------
       * The FC is armed and operational. The main control loop only handles
       * the Control system.
       */
      mpu6050_read_motion( imu_readings );
      get_rate_gyro( param->state, imu_readings );

      Ctrl_Execute(param->ctrl, param->state, param->setpoint, param->control_signal);
      Ctrl_Allocate(param->control_signal, motorSetpoint);
      MotorCtrl_UpdateSetpoint( param->motorControl, motorSetpoint, param->motorControl->nr_init_motors);
      break;
    case state_disarming:
      /*---------------------------------------Disarming mode--------------------------
       * Transitional state. Only executed once.
       */
      MotorCtrl_Disable(param->motorControl);
      Ctrl_Off(param->ctrl);
      if( !State_Change(state_disarmed) )
      {
        State_Fault();
      }
      break;
    case state_fault:
      /*-----------------------------------------fault mode---------------------------
       *  Something has gone wrong and caused the FC to go into fault mode. Nothing
       *  happens in fault mode. Only allowed transition is to disamed mode.
       *
       *  TODO save reason for fault mode.
       */

      break;
    case state_not_available:
      /*-----------------------------------------state not available mode----------------
       * Something is very wrong if the FC is in state_not_available.
       */
      break;
    default:
      break;
    }

    /*------------------------------------read receiver -----------------------------------
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
        translate_receiver_signal_rate( param->setpoint, local_receiver_buffer );
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
          MotorCtrl_Disable(param->motorControl);
          spectrum_receiver_error_counter = 0;
        }
      }
      else
      {
        //Do nothing.
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
      if(State_GetCurrent() != state_disarmed && State_GetCurrent() != state_disarming)
      {
        if(pdTRUE == State_Change(state_disarming))
        {
        }
        else
        {
          //Not allowed to change state.
        }
      }
    }

    /*Arming requested*/
    if ( (local_receiver_buffer->connection_ok)
        && (local_receiver_buffer->ch5 < SATELLITE_CH_CENTER)
        && (local_receiver_buffer->ch0 < 40))
    {
      if(State_GetCurrent() != state_armed && State_GetCurrent() != state_arming)
      {
        if(pdTRUE == State_Change(state_arming))
        {
        }
        else
        {
          //Not allowed to change state.
        }
      }
    }


    vTaskDelayUntil( &xLastWakeTime, mainPeriodTimeMs );

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

