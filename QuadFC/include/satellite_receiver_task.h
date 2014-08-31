/*
 * receiver_task.h
 *
 * Created: 2013-03-24 14:51:05
 *  Author: Martin Lundh
 */

#ifndef SATELLITE_RECEIVER_TASK_H_
#define SATELLITE_RECEIVER_TASK_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "freertos_uart_serial.h"
#include "globals.h"

#define SATELLITE_DATA_MASK 0x7ff            /*Data contained in the 11 least significant bits*/
#define SATELLITE_CHANNEL_MASK 0x7800        /*Channel number contained in the 4 bits above data.*/

#define BAUD_SATELLITE 115200
#define SATELITE_USART USART0

#define SATELLITE_MESSAGE_LENGTH 16

#define SATELLITE_CH_CENTER 1022

/*Receiver queue*/
#define RECEIVER_QUEUE_LENGTH     (2)
#define RECEIVER_QUEUE_ITEM_SIZE        (sizeof(receiver_data_t))

/*Global rtos variable */
extern xQueueHandle xQueue_receiver;

/*
 * Struct containing the data from an RC receiver.
 */
typedef struct receiver_data
{
  int32_t ch0;           //! Channel 0 data.
  int32_t ch1;           //! Channel 1 data.
  int32_t ch2;           //! Channel 2 data.
  int32_t ch3;           //! Channel 3 data.
  int32_t ch4;           //! Channel 4 data.
  int32_t ch5;           //! Channel 5 data.
  int32_t ch6;           //! Channel 6 data.
  int32_t sync;          //! Sync data etc.
  int32_t connection_ok;  //! 0 if ok, != 0 otherwise.
} receiver_data_t;

void create_satellite_receiver_task( void );
void satellite_receiver_task( void *pvParameters );
void satellite_decode_signal( uint8_t data[], receiver_data_t *decoded_data );

#endif /* SATELLITE_RECEIVER_TASK_H_ */
