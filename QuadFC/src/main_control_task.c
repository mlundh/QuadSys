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



void create_main_control_task(void)
{


 
	/*Create the task*/
    portBASE_TYPE create_result;
	create_result = xTaskCreate(    main_control_task,                          /* The function that implements the task.  */
	                (const signed char *const) "Main_Ctrl",                     /* The name of the task. This is not used by the kernel, only aids in debugging*/
	                1000,                                                        /* The stack size for the task*/
	                NULL,                                            /* The already configured motor data is passed as an input parameter*/
	                configMAX_PRIORITIES-1,                                     /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
	                NULL);                                                      /* Handle to the task. Not used here and therefore NULL*/
					
    if (create_result != pdTRUE)
    {
		/*Error - Could not create task. TODO handle this error*/
        for (;;)
        {
            
        }
    }
    
}



/*
 *Task writing whatever is in the queue to the a serial port. The serial port is 
 *passed as an argument to the task. 
*/
void main_control_task(void *pvParameters)
{

	
	static const portTickType xPeriod = (6UL / portTICK_RATE_ONE_THIRD_MS); /*main_control_task execute at 500Hz*/

	
	// declared as public in main_control_task.h
    parameters_angle = pvPortMalloc(sizeof(control_values_pid_t)); /*PID parameters used in angle mode*/
    parameters_rate = pvPortMalloc(sizeof(control_values_pid_t)); /*PID parameters used in rate mode*/

    control_values_pid_t *ctrl_error_angle = pvPortMalloc(sizeof(control_values_pid_t)); /*control error signal used in angle mode*/
    control_values_pid_t *ctrl_error_rate = pvPortMalloc(sizeof(control_values_pid_t)); /*control error signal used in rate mode*/
    control_values_pid_t *ctrl_limit_rate = pvPortMalloc(sizeof(control_values_pid_t)); /*control error signal used in rate mode*/
    
	ctrl_signal = pvPortMalloc(sizeof(control_signal_t));
	
    state = pvPortMalloc(sizeof(state_data_t));   /*State information*/
    setpoint = pvPortMalloc(sizeof(state_data_t)); /*Setpoint information*/
    
    receiver_data_t *local_receiver_buffer = pvPortMalloc(sizeof(receiver_data_t)); /*Receiver information*/
    
	imu_data_t *imu_readings = pvPortMalloc(sizeof(imu_data_t)); /*IMU data struct containing the parsed IMU values.*/
    
	communicaion_packet_t *QSP_log_packet = pvPortMalloc(sizeof(communicaion_packet_t));
	QSP_log_packet->information = pvPortMalloc(sizeof(uint8_t)*MAX_DATA_LENGTH);
	QSP_log_packet->crc = pvPortMalloc(sizeof(uint8_t)*2);
	QSP_log_packet->frame = pvPortMalloc(sizeof(uint8_t)*DISPLAY_STRING_LENGTH_MAX);
	uint8_t *temp_frame_main = pvPortMalloc(sizeof(uint8_t)*DISPLAY_STRING_LENGTH_MAX);
	
	log_parameters = pvPortMalloc(sizeof(int32_t*)*64);
	for (int i = 0; i < 64; i++)
	{
		log_parameters[i] = NULL;
	}
	
	int32_t motor_setpoint[MAX_MOTORS] = { 0 };
	
	xSemaphoreTake(x_param_mutex, portMAX_DELAY);
    uint8_t state_request = 0;
    /*Ensure that all Mallocs returned valid pointers*/
    if(!parameters_angle || !parameters_rate || !ctrl_error_angle || 
			!ctrl_error_rate || !ctrl_limit_rate ||  !state || !setpoint || !local_receiver_buffer
			|| !imu_readings || !x_param_mutex || !ctrl_signal || !temp_frame_main)
    {
        for (;;)
        {
            // TODO error handling! 
        }
    }
	
    quadfc_state_t fc_mode = fc_initializing;
    
    uint8_t reset_integral_error = 0; // 0 == false
     
    // TODO clean up I2C initialization


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

	/*Control parameter initialization for angle mode control.*/	
	parameters_angle->pitch_p = 0;
    parameters_angle->pitch_i = 0;
    parameters_angle->pitch_d = 0;
	parameters_angle->roll_p = 0;
    parameters_angle->roll_i = 0;
    parameters_angle->roll_d = 0;
	parameters_angle->yaw_p = 0;
    parameters_angle->yaw_i = 0;
    parameters_angle->yaw_d = 0;
	parameters_angle->altitude_p = 0;
	parameters_angle->altitude_i = 0;
	parameters_angle->altitude_d = 0;
       
	/*Control parameter initialization for rate mode control.*/
    /*--------------------Initial parameters-------------------*/
	parameters_rate->pitch_p = 180000;
	parameters_rate->pitch_i = 700;
	parameters_rate->pitch_d = 204800;
	parameters_rate->roll_p = 180000;
	parameters_rate->roll_i = 700;
	parameters_rate->roll_d = 204800;
	parameters_rate->yaw_p = 210000;
	parameters_rate->yaw_i = 700;
	parameters_rate->yaw_d = 256000;
	parameters_rate->altitude_p = 1 << SHIFT_EXP;
	parameters_rate->altitude_i = 0;
	parameters_rate->altitude_d = 0;
    
    /*Initialization for control error in angle mode*/
    reset_control_error(ctrl_error_angle);

    
    /*Initialization for control error in rate mode.*/
    reset_control_error(ctrl_error_rate);

    /*Initialization for control error limit in rate mode*/
    ctrl_limit_rate->pitch_p = 1000;
    ctrl_limit_rate->pitch_i = 1000;
    ctrl_limit_rate->pitch_d = 300;
    ctrl_limit_rate->roll_p = 1000;
    ctrl_limit_rate->roll_i = 1000;
    ctrl_limit_rate->roll_d = 300;
    ctrl_limit_rate->yaw_p = 1000;
    ctrl_limit_rate->yaw_i = 2000;
    ctrl_limit_rate->yaw_d = 300;
    ctrl_limit_rate->altitude_p = 0;
    ctrl_limit_rate->altitude_i = 0;
    ctrl_limit_rate->altitude_d = 0;
    
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
	
	
	/*Initialize log parameters*/
	nr_log_parameters = 0;
	
    toggle_pin(13);

    /*Initialize the IMU. A reset followed by a short delay is recommended
     * before starting the configuration.*/
    init_twi_main();
    mpu6050_initialize();

    /*Flight controller should always be started in fc_disarmed mode to prevent
     * unintentional motor arming. */
    fc_mode = fc_disarmed;
    
    toggle_pin(13);


    // TODO read from memory
    uint32_t nr_motors = 4;
    //TODO fcn pointers!
    pwm_init_motor_control(nr_motors);


    uint32_t arming_counter = 0;
    /*The main control loop*/
    unsigned portBASE_TYPE xLastWakeTime = xTaskGetTickCount();
	for (;;)
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
        if (fc_mode == fc_armed_angle_mode)
        {
            //mpu6050_read_motion(MPU6050_RA_ACCEL_XOUT_H, imu_readings, 14);
            //translate_receiver_signal_angle(setpoint, local_receiver_buffer);
            //calc_control_signal_angle_pid(motors, NUMBER_OF_MOTORS, parameters_angle, ctrl_error_angle, state, setpoint, ctrl_signal);
            //mk_esc_write_setpoint_value(motors, NUMBER_OF_MOTORS);
        }
		/*-----------------------------------------Rate mode------------------------------------------------------- 
		 * Rate mode uses only the gyro to stabilize the copter. Only the angular rate is compensated for and the copter
		 * will not self stabilize, rather this control strategy slows the dynamics of the copter down to a level where
		 * a pilot can control them. The setpoint is an angular rate and is stored in setpoint->pitch_rate/roll_rate/yaw_rate. 
		 */
		else if (fc_mode == fc_armed_rate_mode)
		{

			mpu6050_read_motion(imu_readings);
			translate_receiver_signal_rate(setpoint, local_receiver_buffer);
			get_rate_gyro(state, imu_readings);
			calc_control_signal_rate_pid(motor_setpoint, nr_motors, parameters_rate, ctrl_error_rate, state, setpoint, ctrl_signal);
			pwm_update_setpoint(motor_setpoint, nr_motors);
		}
        /*If arming requested, delay to make sure all motors are armed.*/
		else if (fc_mode == fc_arming)
		{
			if(arming_counter >= 1000)
			{
				fc_mode = fc_armed_rate_mode;
				toggle_pin(13);
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
		else if (fc_mode == fc_configure)
		{
			
			/* if a switch back to disarmed mode i requested then switch"*/
			if(xQueueReceive(xQueue_configure_req, &state_request, mainDONT_BLOCK) == pdPASS)
			{
				if (state_request == fc_disarmed)
				{ // TODO is it ok to send here? Possible resource mutex required? 
					QSP_log_packet->address = 2;
					QSP_log_packet->control = ctrl_state_fc_disarmed;
					QSP_log_packet->length = 0;
					encode_QSP_frame(QSP_log_packet, temp_frame_main);
					CommunicationSend(&(QSP_log_packet->frame), QSP_log_packet->frame_length);
					xSemaphoreTake(x_param_mutex, portMAX_DELAY);
					fc_mode = fc_disarmed;
				} // Answer even if asked to change to current state
				else if (state_request == fc_configure)
				{
					QSP_log_packet->address = 2;
					QSP_log_packet->control = ctrl_state_fc_configure;
					QSP_log_packet->length = 0;
					encode_QSP_frame(QSP_log_packet, temp_frame_main);
					CommunicationSend(&(QSP_log_packet->frame), QSP_log_packet->frame_length);
				}

			}
		}     
        /*---------------------------------------Disarmed mode--------------------------------------------
         * When in disarmed mode all integral errors should be reset to avoid integral accumulation.
		 * 
		 * It is only allowed to go into fc_configure mode if the copter is already in fc_disarmed mode.
         */
        
        else if (fc_mode == fc_disarmed)
        {
            if (reset_integral_error)
            {
                reset_control_error(ctrl_error_rate);
                reset_control_error(ctrl_error_angle);
				reset_integral_error = 0;
            }
			/*If a request into fc_configure is sent, switch! */
			if(xQueueReceive(xQueue_configure_req, &state_request, mainDONT_BLOCK) == pdPASS)
			{
				if (state_request == fc_configure)
				{ // TODO is it ok to send here? Possible resource mutex required? 
					QSP_log_packet->address = 2;
					QSP_log_packet->control = ctrl_state_fc_configure;
					QSP_log_packet->length = 0;
					encode_QSP_frame(QSP_log_packet, temp_frame_main);
					CommunicationSend(&(QSP_log_packet->frame), QSP_log_packet->frame_length);
					xSemaphoreGive(x_param_mutex);
					fc_mode = fc_configure;
				}// Answer even if asked to switch to current state
				else if (state_request == fc_disarmed)
				{
					QSP_log_packet->address = 2;
					QSP_log_packet->control = ctrl_state_fc_disarmed;
					QSP_log_packet->length = 0;
					encode_QSP_frame(QSP_log_packet, temp_frame_main);
					CommunicationSend(&(QSP_log_packet->frame), QSP_log_packet->frame_length);
				}
			}

        }
		 
        /*------------------------------------read receiver -----------------------------------------------
         *
         *
         */
        if (xQueueReceive(xQueue_receiver, local_receiver_buffer, mainDONT_BLOCK) == pdPASS)
        {
			/* The code in this scope will execute once every 22 ms (the frequency of new data
			 * from the receiver).
			 */
        }
        
		/*--------------- If there is no connection to the receiver, put FC in disarmed mode-------------*/
        if ((!local_receiver_buffer->connection_ok) && (fc_mode != fc_configure) && (fc_mode != fc_disarmed))
        {
            fc_mode = fc_disarmed; /*Error - no connection, TODO do something!*/
            reset_integral_error = 1;
            pwm_dissable();
        }

     
		/*-------------------------------------State change request from receiver?----------------------------
         * Check if a fc_state change is requested, if so, change state. State change is only allowed if the copter is landed. 
         */

        /*Disarm requested*/
		if ((local_receiver_buffer->connection_ok) && (local_receiver_buffer->ch5 > SATELLITE_CH_CENTER) && 
			(local_receiver_buffer->ch0 < 40) && (fc_mode != fc_disarmed) && (fc_mode != fc_configure))
		{
    		fc_mode = fc_disarmed;
            reset_integral_error = 1;
            pwm_dissable();
		}
		/*Arming requested*/
		if ((local_receiver_buffer->connection_ok) && (local_receiver_buffer->ch5 < SATELLITE_CH_CENTER) && 
			(local_receiver_buffer->ch0 < 40) && (fc_mode != fc_armed_rate_mode) && (fc_mode != fc_configure) && (fc_mode != fc_arming))
		{
    		fc_mode = fc_arming;
    		arming_counter = 0;
    		toggle_pin(13);
    		pwm_enable(nr_motors);
		}
        
		/*------------------------------------------- Logging? ------------------------------------------ 
		 * If the FC is in a flight state and logging is requested, log data!
		 */
		if (nr_log_parameters && (fc_mode != fc_disarmed) && (fc_mode != fc_configure))
		{
			if ((uxQueueMessagesWaiting(xQueue_display) < DISPLAY_QUEUE_LENGTH) && (xSemaphoreTake(x_log_mutex, 0) == pdPASS))
			{
				time_main = xTaskGetTickCount();
				do_logging(QSP_log_packet);	
				QSP_log_packet->address = 2;
				QSP_log_packet->control = data_log;
				QSP_log_packet->length = nr_log_parameters * sizeof(uint32_t);
				encode_QSP_frame(QSP_log_packet, temp_frame_main);
				CommunicationSend(&(QSP_log_packet->frame), QSP_log_packet->frame_length);
			}
		}





        vTaskDelayUntil(&xLastWakeTime,xPeriod);
        
        /*-------------------Heartbeat----------------------
        * Toggle an led to indicate that the FC is operational.
        */
		toggle_pin(33);
        /*-------------------Heartbeat-----------------*/
	}
	/* The task should never escape the for-loop and therefore never return.*/

}


/* Simply resets the control error (or any control_values_pid_t struct passed to it).*/
void reset_control_error(control_values_pid_t *ctrl_error)
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

/* Collects data for logging. Logging must be setup by a call to WriteLogParameters().*/
void do_logging(communicaion_packet_t *packet)
{
	uint8_t *current_log_param;
	for (int i = 0; i < nr_log_parameters; i++)
	{
		/* log_parameters is a pointer to pointers to int32_t,
		 * the following code casts, in turn, each of those pointers
		 * to a uint8_t pointer. This pointer is used to do a deep copy  
		 * of the desired raw data into the packet.*/
		current_log_param = (uint8_t *)(log_parameters[i]); 
		for (int k = 0; k < 4; k++)
		{
			/*Deep copy the desired log data into the log packet.*/
			packet->information[i*4 + k] = (current_log_param[k]);
		}
	}
	
	
}



void init_twi_main()
{
    freertos_peripheral_options_t async_driver_options = {
        NULL,											                        /* This peripheral does not need a receive buffer, so this parameter is just set to NULL. */
        0,												                        /* There is no Rx buffer, so the rx buffer size is not used. */
        (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1),	                        /* The priority used by the TWI interrupts. It is essential that the priority does not have a numerically lower value than configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY*/
        TWI_I2C_MASTER,									                        /* TWI is configured as an I2C master. */
        0,																		/* The asynchronous driver is used, so WAIT_TX_COMPLETE and WAIT_RX_COMPLETE are not set. */
    };

	freertos_peripheral_options_t async_driver_options1 = {
		NULL,											                        /* This peripheral does not need a receive buffer, so this parameter is just set to NULL. */
		0,												                        /* There is no Rx buffer, so the rx buffer size is not used. */
		(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1),	                        /* The priority used by the TWI interrupts. It is essential that the priority does not have a numerically lower value than configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY*/
		TWI_I2C_MASTER,									                        /* TWI is configured as an I2C master. */
		0,																		/* The asynchronous driver is used, so WAIT_TX_COMPLETE and WAIT_RX_COMPLETE are not set. */
	};


    freertos_twi_if twi_0 = freertos_twi_master_init(TWI0, &async_driver_options);		/*Initialize twi bus 0, available at pin xx and xx on the Arduino due*/
    freertos_twi_if twi_1 = freertos_twi_master_init(TWI1, &async_driver_options1);		/*Initialize twi bus 1, available at pin xx and xx on the Arduino due*/

    if (twi_0 == NULL || twi_1 == NULL)
    {
        for (;;)
        {
            // ERROR, TODO set error flag and halt execution*/
        }
    }


    twi_set_speed(TWI0, 400000, sysclk_get_cpu_hz());                       /*High speed TWI setting*/
    twi_set_speed(TWI1, 400000, sysclk_get_cpu_hz());                       /*High speed TWI setting*/
}
