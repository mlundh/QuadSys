/*
 * Test_task.h
 *
 * Created: 2013-02-09 14:01:38
 *  Author: Martin Lundh
 *
 *  Data is stored according to:
 *
 *  [start byte][address][control][length][(re-)start byte][data (0-MAX_DATA_LENGTH bytes)][crc][end byte]
 *
 *  and the start byte is escaped according to QSP_control_octets enum.
 *
 *
 */

#ifndef COMMUNICATION_TASKS_H_
#define COMMUNICATION_TASKS_H_

#include "crc.h"
#include "control_algorithm.h"
#include "stdint.h"
/* Atmel library includes. */
#include "freertos_uart_serial.h"
/* Use USART1 (labeled RX2 17 and TX2 16)*/
#define TERMINAL_USART USART1

/*---------------------------------------------------------*/
/*
 * Struct containing the data protocol.
 * | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *
 * +-------------------------------+
 * |            startbyte          |
 * +-------------------------------+
 * |    address    |   control     |
 * +---------------+---------------+
 * |   payload length (0 to 31)    |
 * +-------------------------------+
 * |            restart            |
 * +-------------------------------+
 * |       payload    (0 to 31)    |
 * +-------------------------------+
 * |             crc               |
 * +-------------------------------+
 * |          stopbyte             |
 * +-------------------------------+
 *
 * frame format:
 * [start][address][control][length][information][crc][stop]
 */
#define NR_OVERHEAD_BYTES 7
#define MAX_DATA_LENGTH 58
typedef struct communicaion_packet
{
  uint8_t address;
  uint8_t control;
  uint8_t length;
  uint8_t *information;
  uint8_t *crc;
  uint8_t *serialized_frame;
  uint8_t serialized_frame_length;
} communicaion_packet_t;

/**
 * enum describing valid control bytes.
 */
typedef enum QSP_control
{
  test = 0,

  Log_query           = 1,        // Query the available log variables
  Log_set             = 2,        // Set log variables to be logged
  Log_start           = 3,        // Start logging
  Log_stop            = 4,        // Stop logging
  State_query         = 5,        // Query fc state
  State_set           = 6,        // Set fc state
  Param_query         = 7,        // Query the available parameters
  Paream_set          = 8,        // Set parameters.

  ctrl_req_pending = 246,
  ctrl_state_fc_disarmed = 247,
  ctrl_state_fc_configure = 248,
  ctrl_req_state_fc_configure = 249,
  ctrl_req_state_fc_disarmed = 250,
  ctrl_crc_error = 251,             // Previous frame had an invalid CRC, bad transmission, retransmit.
  ctrl_invalid_command = 252,       // previous command was not valid.
  ctrl_ack = 253,                   // Send acknowledge to the caller.
  ctrl_echo = 254                   // Echo the frame back to the caller.
} QSP_control_t;

/*Max length of SLIP frame*/
#define SLIP_MAX_LENGTH (128)


/*Display queue*/
#define DISPLAY_QUEUE_LENGTH      (1)
#define DISPLAY_QUEUE_ITEM_SIZE         (sizeof(uint8_t *))
#define DISPLAY_STRING_LENGTH_MAX       128
extern  QueueHandle_t xQueue_display;

/*Display queue*/
#define DISPLAY_NR_BYTES_QUEUE_LENGTH     (1)
#define DISPLAY_NR_BYTES_QUEUE_ITEM_SIZE         (sizeof(uint8_t))
#define DISPLAY_NR_BYTES_STRING_LENGTH_MAX       128
extern  QueueHandle_t xQueue_display_bytes_to_send;


void Com_CreateCommunicationTasks( void );

void Com_TxTask( void *pvParameters );
void Com_RxTask( void *pvParameters );
void Com_CommunicationSend( const void * const pvItemToQueue, uint8_t bytesToSend );
void Com_HandleCommunication( communicaion_packet_t *packet);
void Com_SendAck( communicaion_packet_t *QSP_packet);

uint8_t Com_SerializeQSPFrame( communicaion_packet_t *packet);
uint8_t Com_DeserializeQSPFrame( communicaion_packet_t *packet);



#endif /* COMMUNICATION-TASKS-H- */
