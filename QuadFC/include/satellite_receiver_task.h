/*
 * receiver_task.h
 *
 * Created: 2013-03-24 14:51:05
 *  Author: Martin Lundh
 */ 


#ifndef SATELLITE_RECEIVER_TASK_H_
#define SATELLITE_RECEIVER_TASK_H_

#include "freertos_uart_serial.h"
#include "main_control_task.h"

#define SATELLITE_DATA_MASK 0x7ff            /*Data contained in the 11 least significant bits*/
#define SATELLITE_CHANNEL_MASK 0x7800        /*Channel number contained in the 4 bits above data.*/

#define BAUD_SATELLITE 115200
#define SATELITE_USART USART0

#define SATELLITE_MESSAGE_LENGTH 16

#define SATELLITE_CH_CENTER 1022


void create_satellite_receiver_task(void);
void satellite_receiver_task(void *pvParameters);
void satellite_decode_signal(uint8_t data[], receiver_data_t *decoded_data);

#endif /* SATELLITE_RECEIVER_TASK_H_ */
