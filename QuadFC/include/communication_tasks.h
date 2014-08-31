/*
 * Test_task.h
 *
 * Created: 2013-02-09 14:01:38
 *  Author: Martin Lundh
 *
 *  Data is stored according to:
 *
 *  [start byte][address][control][length][data (0-MAX_DATA_LENGTH bytes)][crc][end byte]
 *
 *
 *
 *
 */

#ifndef COMMUNICATION_TASKS_H_
#define COMMUNICATION_TASKS_H_

#include "crc.h"
#include "control_algorithm.h"
#include "compiler.h"
/* Atmel library includes. */
#include "freertos_uart_serial.h"
/* Use USART1 (labeled RX2 17 and TX2 16)*/
#define TERMINAL_USART USART1

/*---------------------------------------------------------*/
/*
 * Struct containing the data protocol.
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
  uint8_t *frame;
  uint8_t frame_length;
} communicaion_packet_t;

typedef enum QSP_control_octets
{
  frame_boundary_octet = 0x7E,
  frame_boundary_octet_replacement = 0x5E,
  control_escape_octet = 0x7D,
  control_escape_octet_replacement = 0x5D
} QSP_control_octets_t;

typedef enum QSP_control
{
  test = 0,

  // Request commands do not carry any data. They should allways be followed by a response
  // from the remote party.
  req_pid_rate_param = 1,     // Request PID rate parameters.
  req_pid_angle_param = 2,    // Request PID angle parameters.
  req_pid_rate_limit = 3,     // Request PID rate limits.
  req_pid_angle_limit = 4,    // Request PID angle limits.
  req_logging_param = 5, // Request what parameters are beeing logged at the moment.
  req_logg_start = 6,         // Request logging to begin.
  req_fc_state = 7,           // Request Flight controller state.
  req_placeholder_4 = 8,
  req_placeholder_5 = 9,
  req_placeholder_6 = 10,
  req_placeholder_7 = 11,
  req_placeholder_8 = 12,
  req_placeholder_9 = 13,
  req_placeholder_11 = 15,
  req_placeholder_12 = 16,
  req_placeholder_13 = 17,
  req_placeholder_14 = 18,

  // Data commands carry data, they may travell both ways and do not
  // allways need to be preceded by a request.
  data_pid_rate_param = 21,       // Deliver PID rate parameters.
  data_pid_angle_param = 22,      // Deliver PID angle parameters.
  data_pid_rate_limit = 23,       // Deliver PID rate limits.
  data_pid_angle_limit = 24,      // Deliver PID angle limits.
  data_logging_param = 25,        // Deliver Logging parameters.
  data_log = 26,                 // Deliver logg data.
  data_fc_state = 27,             // Deliver flight controller state.
  data_placeholder_4 = 28,
  data_placeholder_5 = 29,
  data_placeholder_6 = 30,
  data_placeholder_7 = 31,
  data_placeholder_8 = 32,
  data_placeholder_9 = 33,
  data_placeholder_11 = 35,
  data_placeholder_12 = 36,
  data_placeholder_13 = 37,
  data_placeholder_14 = 38,
  data_placeholder_15 = 39,
  data_placeholder_16 = 20,

  ctrl_req_pending = 246,
  ctrl_state_fc_disarmed = 247,
  ctrl_state_fc_configure = 248,
  ctrl_req_state_fc_configure = 249,
  ctrl_req_state_fc_disarmed = 250,
  ctrl_crc_error = 251, // Previous frame had an invald CRC, bad transmission, retransmitt.
  ctrl_invalid_command = 252,  // previous command was not valid.
  ctrl_ack = 253,              // Send acknoledge to the caller.
  ctrl_echo = 254              // Echo the frame back to the caller.
} QSP_control_t;

/*Req configure queue*/
#define CONFIGURE_REQ_QUEUE_LENGTH      (1)
#define CONFIGURE_REQ_QUEUE_ITEM_SIZE        (sizeof(uint8_t))
extern xQueueHandle xQueue_configure_req;

/*Display queue*/
#define DISPLAY_QUEUE_LENGTH      (1)
#define DISPLAY_QUEUE_ITEM_SIZE         (sizeof(uint8_t *))
#define DISPLAY_STRING_LENGTH_MAX       128
extern xQueueHandle xQueue_display;

/*Display queue*/
#define DISPLAY_NR_BYTES_QUEUE_LENGTH     (1)
#define DISPLAY_NR_BYTES_QUEUE_ITEM_SIZE         (sizeof(uint8_t))
#define DISPLAY_NR_BYTES_STRING_LENGTH_MAX       128
extern xQueueHandle xQueue_display_bytes_to_send;


void create_communication_tasks( void );

void tx_task( void *pvParameters );
void rx_task( void *pvParameters );
void CommunicationSend( const void * const pvItemToQueue, uint8_t bytesToSend );
void ReadControlValuesPid( uint8_t *raw_data, control_values_pid_t *pidValues );
void WriteControlValuesPid( uint8_t *raw_data, control_values_pid_t *pidValues );
void HandleCommunication( communicaion_packet_t *packet, uint8_t* temp_frame );
void SendAck( communicaion_packet_t *QSP_packet, uint8_t* temp_frame );
void WriteLogParameters( uint8_t *raw_data );
uint8_t decode_QSP_frame( communicaion_packet_t *packet, uint8_t* temp_frame );
uint8_t encode_QSP_frame( communicaion_packet_t *packet, uint8_t* temp_frame );



#endif /* COMMUNICATION-TASKS-H- */
