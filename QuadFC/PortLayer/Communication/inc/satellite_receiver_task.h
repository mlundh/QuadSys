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
#include "Utilities/inc/globals.h"

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
extern  QueueHandle_t xQueue_receiver;

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

#define CHANNEL_CONFIDENCE 1
#define FRAME_CONFIDENCE_MAX 15
#define FRAME_CONFIDENCE_THRESHOLD 10

/*
 * Struct containing the data from a spektrum satellite RC receiver.
 *
*/

typedef struct spektrum_channel {
  uint16_t value;
} spektrum_channel_t;

typedef struct spektrum_frame {
  uint16_t channels;
  uint8_t confidence;
} spektrum_frame_t;

typedef struct spektrum_data {
    spektrum_channel_t ch[15]; 
    uint8_t ch_count;
    spektrum_frame_t frame_type[3];
    uint8_t frame_type_count;
    bool confident_frame_types;
    uint16_t channels_available;
    uint16_t channels_last_read;
} spektrum_data_t;

typedef struct spektrum_channels {
  spektrum_channel_t ch[15];
  uint8_t ch_count;
  uint16_t channels_merged;
  uint8_t frames_needed;
  bool updated;
} spektrum_channels_t;

typedef struct fc_channel {
  uint16_t value;
  bool updated;
} fc_channel_t;

typedef struct fc_channels {
  fc_channel_t stick_left_up_down;
  fc_channel_t stick_left_right_left;
  fc_channel_t stick_right_up_down;
  fc_channel_t stick_right_right_left;
  fc_channel_t aux[10];
  uint8_t aux_channel_count;
  bool updated;
} fc_channels_t;


void create_satellite_receiver_task( void );
void satellite_receiver_task( void *pvParameters );
int satellite_decode_signal(uint8_t data[], spektrum_data_t *decoded_data);
fc_channels_t dx7s_to_fc_channels(spektrum_channels_t *reciever_channels);
receiver_data_t dx7s_to_legacy_fc_channels(spektrum_channels_t *reciever_channels);

#endif /* SATELLITE_RECEIVER_TASK_H_ */
