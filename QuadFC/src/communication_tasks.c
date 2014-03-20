/*
 * communication_tasks.c
 *
 *
 * Copyright (C) 2014  Martin Lundh
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "main.h"
#include "communication_tasks.h"

/*
 * create_communication_task
 *
 * Create the task responsible for the HMI.
 * 
 */ 
void create_communication_tasks(void)
{
	
    
	uint8_t* receive_buffer_driver = pvPortMalloc(sizeof(uint8_t) * DISPLAY_STRING_LENGTH_MAX);
	freertos_usart_if freertos_usart;
	freertos_peripheral_options_t driver_options = {
		receive_buffer_driver,						    /* The buffer used internally by the USART driver to store incoming characters. */
		DISPLAY_STRING_LENGTH_MAX,						/* The size of the buffer provided to the USART driver to store incoming characters. */
		(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 4),	/* The priority used by the USART interrupts. */
		USART_RS232,									/* Configure the USART for RS232 operation. */
		(WAIT_TX_COMPLETE)								/* Wait for a Tx to complete before returning from send functions. */
	};

	const sam_usart_opt_t usart_settings = {
		57600,                                          /* Speed of the transfer, baud*/
		US_MR_CHRL_8_BIT,                               /* 8 bit transfer*/
		US_MR_PAR_NO,                                   /* No parity*/
		US_MR_NBSTOP_1_BIT,                             /* One stop bit*/
		US_MR_CHMODE_NORMAL,                            /* */
		0                                               /* Only used in IrDA mode. */
	};

	/* Initialize the USART interface. */
	freertos_usart = freertos_usart_serial_init(TERMINAL_USART,
												&usart_settings,
												&driver_options);
	
	xTaskCreate(	tx_task,					    /* The task that implements the test. */
					(const int8_t *const) "UARTTX",	    /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
					500,							    /* The size of the stack allocated to the task. */
					(void *) freertos_usart,		    /* The parameter is used to pass the already configured USART port into the task. */
					configMAX_PRIORITIES-3,			    /* The priority allocated to the task. */
		    		NULL);							    /* Used to store the handle to the created task - in this case the handle is not required. */
                    
	xTaskCreate(	rx_task,					    /* The task that implements the test. */
	                (const int8_t *const) "UARTRX",	    /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
	                500,							    /* The size of the stack allocated to the task. */
	                (void *) freertos_usart,		    /* The parameter is used to pass the already configured USART port into the task. */
	                configMAX_PRIORITIES-3,			    /* The priority allocated to the task. */
	                NULL);							    /* Used to store the handle to the created task - in this case the handle is not required. */
}



/*
 * Task writing whatever is in the queue to the a serial port. The serial port is 
 * passed as an argument to the task. 
*/
void tx_task(void *pvParameters)
{
	
	
	portTickType max_block_time_ticks = 100UL / portTICK_RATE_ONE_THIRD_MS;
	status_code_t result;
	uint8_t *string_ptr;
    uint8_t nr_bytes_to_send;
	/*The already open usart port is passed through the task parameters*/
	freertos_usart_if xbee_usart = (freertos_usart_if)pvParameters;
   
    
	
	for ( ;; )
	{ 

        /*Wait blocking for items in the queue and transmit the data
        *as soon as the data arrives*/
		
        xQueuePeek(xQueue_display, &string_ptr, portMAX_DELAY);
        xQueuePeek(xQueue_display_bytes_to_send, &nr_bytes_to_send, portMAX_DELAY);
		
		
        if (nr_bytes_to_send < DISPLAY_STRING_LENGTH_MAX)
        {
            
            result = freertos_usart_write_packet(   xbee_usart,     /* The USART port*/
                                                (string_ptr),  /* The local buffer holding data to be transmitted.*/
                                                nr_bytes_to_send,              /* Length in bytes of the data to be transmitted*/
                                                max_block_time_ticks);  /* Block time*/
                        
            if (result != STATUS_OK)
            {
                //error vtasksuspend?
            }
                        
                        
        }
		else
		{
			//error!
		}

		if (xQueueReceive(xQueue_display_bytes_to_send, &nr_bytes_to_send, 0) != pdPASS || xQueueReceive(xQueue_display, &string_ptr, 0) != pdPASS)
		{
			/*Error, should never come here, if Peek is successful then receive should always work.*/
		}
		

	}

}


void rx_task(void *pvParameters)
{
	
	int k = 0;
    uint32_t bytes_received;
    uint8_t *receive_buffer = pvPortMalloc(sizeof(uint8_t) * DISPLAY_STRING_LENGTH_MAX);
	
    communicaion_packet_t *QSP_packet = pvPortMalloc(sizeof(communicaion_packet_t));
	QSP_packet->information = pvPortMalloc(sizeof(uint8_t)*MAX_DATA_LENGTH);
    QSP_packet->crc = pvPortMalloc(sizeof(uint8_t)*2);
	QSP_packet->frame = pvPortMalloc(sizeof(uint8_t)*DISPLAY_STRING_LENGTH_MAX);
	
	uint8_t *temp_frame = pvPortMalloc(sizeof(uint8_t) * DISPLAY_STRING_LENGTH_MAX);
    crcTable = pvPortMalloc(sizeof(uint16_t) * 256);
	uint8_t state_request;
	if (!QSP_packet || !QSP_packet || !QSP_packet->information || !QSP_packet->crc || !QSP_packet->frame || !crcTable)
	{
		for (;;)
		{
			// Error! 
		}
	}
	static const portTickType xBlockTime = (30UL / portTICK_RATE_ONE_THIRD_MS); /*Wait for semaphore max 10ms*/
    crcInit();
    /*The already open usart port is passed through the task parameters*/
    freertos_usart_if xbee_usart = (freertos_usart_if)pvParameters;
    
    
    for ( ;; )
    {
		
        /*--------------------------Receive the frame---------------------
         * The frame length can vary, one has to listen to start and stop flags.*/

        bytes_received = freertos_usart_serial_read_packet(   xbee_usart,   /* The USART port*/
                                                    receive_buffer,         /* Pointer to the buffer into which received data is to be copied.*/
                                                    1,						/* The number of bytes to copy. */
                                                    portMAX_DELAY);  /* Block time*/
		
													
        if (bytes_received >= 1)
        {
			
			for (int i = 0; i < bytes_received; i++, k++)
			{
				// TODO Figure out why there is a delay
				QSP_packet->frame[k] = receive_buffer[i];
				if (QSP_packet->frame[k] == frame_boundary_octet)
				{
					if (k > 4)
					{
						QSP_packet->frame_length = k;
						// Decode the message.
						decode_QSP_frame(QSP_packet, temp_frame);
						
						
						// If the packet contains a request to change flight mode, try doing so.
						switch (QSP_packet->control)
						{
							// if we want a state change:
							case ctrl_req_state_fc_configure:
							
								state_request = fc_configure;
								if (xQueueSendToBack(xQueue_configure_req, &state_request , mainDONT_BLOCK) != pdPASS)
								{
									QSP_packet->address = 2;
									QSP_packet->control = ctrl_req_pending;
									QSP_packet->length = 0;
									encode_QSP_frame(QSP_packet, temp_frame);
									CommunicationSend(&(QSP_packet->frame), QSP_packet->frame_length);
								}
								
							break;
							case ctrl_req_state_fc_disarmed:
							
								state_request = fc_disarmed;
								if (xQueueSendToBack(xQueue_configure_req, &state_request, mainDONT_BLOCK) != pdPASS)
								{
									QSP_packet->address = 2;
									QSP_packet->control = ctrl_req_pending;
									QSP_packet->length = 0;
									encode_QSP_frame(QSP_packet, temp_frame);
									CommunicationSend(&(QSP_packet->frame), QSP_packet->frame_length);
								}
								
								
							break;
							case ctrl_ack: /*Simple flow control*/
								xSemaphoreGive(x_log_mutex);
								
							break;
							default:
							// else, acquire mutex and handle communication.
								if (xSemaphoreTake(x_param_mutex, xBlockTime) == pdPASS)
								{
									HandleCommunication(QSP_packet, temp_frame);
									// Give mutex! 
									if (xSemaphoreGive(x_param_mutex) == pdPASS)
									{
										// should not fail!
									}
								}
								else // Mutex not acquired
								{
									QSP_packet->address = 2;
									QSP_packet->control = ctrl_invalid_command;
									QSP_packet->length = 0;
									encode_QSP_frame(QSP_packet, temp_frame);
									CommunicationSend(&(QSP_packet->frame), QSP_packet->frame_length);
								}
							break;
						}

						
						
						
					}
					else // (QSP_packet->frame[k] == frame_boundary_octet)
					{
						QSP_packet->frame[0] = receive_buffer[i];
					}
					// i 
					k = 0;
				}
			}
			
			
		
		}
        else //if (bytes_received >= 1)
        {
            //Error!
        }

        

    }
    
}        
void SendAck(communicaion_packet_t *QSP_packet, uint8_t* temp_frame)
{
		QSP_packet->address = 2;
		QSP_packet->control = ctrl_ack;
		QSP_packet->length = 0;
		encode_QSP_frame(QSP_packet, temp_frame);
		CommunicationSend(&(QSP_packet->frame), QSP_packet->frame_length);
}


void HandleCommunication(communicaion_packet_t *QSP_packet, uint8_t* temp_frame)
{
	switch (QSP_packet->control)
	{
		case data_pid_rate_param:
			// Write new data to the struct parameters_rate
			WriteControlValuesPid(QSP_packet->information, parameters_rate);
			// Acknowledge by returning the current parameters.
			ReadControlValuesPid(QSP_packet->information, parameters_rate);
			QSP_packet->address = 2;
			QSP_packet->control = data_pid_rate_param;
			QSP_packet->length = 36;
			encode_QSP_frame(QSP_packet, temp_frame);
			CommunicationSend(&(QSP_packet->frame), QSP_packet->frame_length);
		break;
		case data_pid_angle_param:
									
		break;
		case data_pid_rate_limit:
									
		break;
		case data_pid_angle_limit:
									
		break;
		case  data_logging_param:
			WriteLogParameters(QSP_packet->information);
		break;
		case req_pid_rate_param:
			ReadControlValuesPid(QSP_packet->information, parameters_rate);
			QSP_packet->address = 2;
			QSP_packet->control = data_pid_rate_param;
			QSP_packet->length = 36;
			encode_QSP_frame(QSP_packet, temp_frame);
			CommunicationSend(&(QSP_packet->frame), QSP_packet->frame_length);
									
		break;
		case req_pid_angle_param:
									
		break;
		case req_pid_rate_limit:
									
		break;
		case req_pid_angle_limit:
									
		break;

		default:
		/* Your code here */
		break;
	}

}



void CommunicationSend(const void * const pvItemToQueue, uint8_t bytesToSend)
{
    //TODO add check to make sure sending was successful. 
    xQueueSendToBack(xQueue_display, pvItemToQueue, mainDONT_BLOCK);
    xQueueSendToBack(xQueue_display_bytes_to_send, &bytesToSend, mainDONT_BLOCK);
    
}



void WriteControlValuesPid(uint8_t *raw_data, control_values_pid_t *pidValues)
{
    int32_t * data = (int32_t *)raw_data;
    pidValues->pitch_p = data[0];
    pidValues->pitch_i = data[1];
    pidValues->pitch_d = data[2];
    pidValues->roll_p = data[3];
    pidValues->roll_i = data[4];
    pidValues->roll_d = data[5];
    pidValues->yaw_p = data[6];
    pidValues->yaw_i = data[7];
    pidValues->yaw_d = data[8];
    pidValues->altitude_p = data[9];
    pidValues->altitude_i = data[10];
    pidValues->altitude_d = data[11];
}

void ReadControlValuesPid(uint8_t *raw_data, control_values_pid_t *pidValues)
{
	int32_t * data = (int32_t *)raw_data;
	data[0] = pidValues->pitch_p;
	data[1] = pidValues->pitch_i;
	data[2] = pidValues->pitch_d;
	data[3] = pidValues->roll_p;
	data[4] = pidValues->roll_i; 
	data[5] = pidValues->roll_d;
	data[6] = pidValues->yaw_p;
	data[7] = pidValues->yaw_i;
	data[8] = pidValues->yaw_d;
	data[9] = pidValues->altitude_p;
	data[10] = pidValues->altitude_i;
	data[11] = pidValues->altitude_d;
}

void WriteLogParameters(uint8_t *raw_data)
{
	int i = 0;
	uint32_t *raw_casted = (uint32_t *)raw_data;
	if (raw_casted[0] & time)
	{
		log_parameters[i++] = &time_main;
	}
	if (raw_casted[0] & pitch_)
	{
		log_parameters[i++] = &(state->pitch);
	}
	if (raw_casted[0] & roll_)
	{
		log_parameters[i++] = &(state->roll);
	}
	if (raw_casted[0] & yaw_)
	{
		log_parameters[i++] = &(state->yaw);
	}
	if (raw_casted[0] & pitch_rate_)
	{
		log_parameters[i++] = &(state->pitch_rate);
	}
	if (raw_casted[0] & roll_rate_)
	{
		log_parameters[i++] = &(state->roll_rate);
	}
	if (raw_casted[0] &yaw_rate_)
	{
		log_parameters[i++] = &(state->yaw_rate);
	}
	if (raw_casted[0] &x_pos_)
	{
		log_parameters[i++] = &(state->x_pos);
	}
	if (raw_casted[0] &y_pos_)
	{
		log_parameters[i++] = &(state->y_pos);
	}
	if (raw_casted[0] &z_pos_)
	{
		log_parameters[i++] = &(state->z_pos);
	}
	if (raw_casted[0] &x_vel_)
	{
		log_parameters[i++] = &(state->x_vel);
	}
	if (raw_casted[0] &y_vel_)
	{
		log_parameters[i++] = &(state->y_vel);
	}
	if (raw_casted[0] &z_vel_)
	{
		log_parameters[i++] = &(state->z_vel);
	}
	/**/
	if (raw_casted[0] & ctrl_torque_pitch)
	{
		log_parameters[i++] = &(ctrl_signal->torque_pitch);
	}
	if (raw_casted[0] & ctrl_torque_roll)
	{
		log_parameters[i++] = &(ctrl_signal->torque_roll);
	}
	if (raw_casted[0] & ctrl_torque_yaw)
	{
		log_parameters[i++] = &(ctrl_signal->torque_yaw);
	}
	if (raw_casted[0] & ctrl_thrust)
	{
		log_parameters[i++] = &(ctrl_signal->thrust);
	}

	nr_log_parameters = i;
}
