/*
 * communication_tasks.c
 *
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
#include "communication_tasks.h"
#include "led_control_task.h"
#include "globals.h"
#include "main_control_task.h"
#include "crc.h"
#include "slip_utils.h"

void Com_CreateCommunicationTasks( void )
{

  uint8_t* receive_buffer_driver = pvPortMalloc(
      sizeof(uint8_t) * DISPLAY_STRING_LENGTH_MAX );
  freertos_usart_if freertos_usart;
  freertos_peripheral_options_t driver_options = { receive_buffer_driver,   /* The buffer used internally by the USART driver to store incoming characters. */
      DISPLAY_STRING_LENGTH_MAX,                                            /* The size of the buffer provided to the USART driver to store incoming characters. */
      (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 4),                   /* The priority used by the USART interrupts. */
      USART_RS232,                                                          /* Configure the USART for RS232 operation. */
      (WAIT_TX_COMPLETE)                                                    /* Wait for a Tx to complete before returning from send functions. */
  };

  const sam_usart_opt_t usart_settings = { 57600,                           /* Speed of the transfer, baud*/
      US_MR_CHRL_8_BIT,                                                     /* 8 bit transfer*/
      US_MR_PAR_NO,                                                         /* No parity*/
      US_MR_NBSTOP_1_BIT,                                                   /* One stop bit*/
      US_MR_CHMODE_NORMAL,                                                  /* */
      0                                                                     /* Only used in IrDA mode. */
  };

  /* Initialize the USART interface. */
  freertos_usart = freertos_usart_serial_init( TERMINAL_USART, &usart_settings,
      &driver_options );

  xTaskCreate( Com_TxTask,                              /* The task that implements the test. */
      (const char *const) "UARTTX",                /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
      500,                                           /* The size of the stack allocated to the task. */
      (void *) freertos_usart,                       /* The parameter is used to pass the already configured USART port into the task. */
      configMAX_PRIORITIES-3,                        /* The priority allocated to the task. */
      NULL );                                        /* Used to store the handle to the created task - in this case the handle is not required. */

  xTaskCreate( Com_RxTask,                              /* The task that implements the test. */
      (const char *const) "UARTRX",                /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
      500,                                           /* The size of the stack allocated to the task. */
      (void *) freertos_usart,                       /* The parameter is used to pass the already configured USART port into the task. */
      configMAX_PRIORITIES-3,                        /* The priority allocated to the task. */
      NULL );                                        /* Used to store the handle to the created task - in this case the handle is not required. */
}

/*
 * Task writing whatever is in the queue to the a serial port. The serial port is
 * passed as an argument to the task.
 */
void Com_TxTask( void *pvParameters )
{

  TickType_t max_block_time_ticks = 333UL / portTICK_PERIOD_MS;
  status_code_t result;
  uint8_t *string_ptr;
  uint8_t nr_bytes_to_send;
  /*The already open usart port is passed through the task parameters*/
  freertos_usart_if xbee_usart = (freertos_usart_if) pvParameters;

  for ( ;; )
  {

    /*Wait blocking for items in the queue and transmit the data
     *as soon as the data arrives*/

    xQueuePeek( xQueue_display, &string_ptr, portMAX_DELAY );
    xQueuePeek( xQueue_display_bytes_to_send, &nr_bytes_to_send,portMAX_DELAY );

    if ( nr_bytes_to_send < DISPLAY_STRING_LENGTH_MAX )
    {
      // TX is blocking
      result = freertos_usart_write_packet( xbee_usart,
        (string_ptr),
        nr_bytes_to_send,
        max_block_time_ticks );
      if ( result != STATUS_OK )
      {
        //error vtasksuspend?
      }

    }
    else
    {
      //error!
    }

    if ( xQueueReceive( xQueue_display_bytes_to_send, &nr_bytes_to_send, 0 )
        != pdPASS || xQueueReceive(xQueue_display, &string_ptr, 0) != pdPASS )
    {
      /*Error, should never come here, if Peek is successful then receive should always work.*/
    }

  }

}

void Com_RxTask( void *pvParameters )
{

  int k = 0;

  uint8_t *receive_buffer = pvPortMalloc(sizeof(uint8_t) * DISPLAY_STRING_LENGTH_MAX );

  communicaion_packet_t *QSP_packet = pvPortMalloc(sizeof(communicaion_packet_t) );
  QSP_packet->information = pvPortMalloc( sizeof(uint8_t) * MAX_DATA_LENGTH );
  QSP_packet->crc = pvPortMalloc( sizeof(uint8_t) * 2 );
  QSP_packet->serialized_frame = pvPortMalloc(sizeof(uint8_t) * DISPLAY_STRING_LENGTH_MAX );

  uint8_t *slip_frame = pvPortMalloc(sizeof(uint8_t) * DISPLAY_STRING_LENGTH_MAX );
  uint8_t slip_frame_length = 0;

  crcTable = pvPortMalloc( sizeof(uint16_t) * 256 );


  if ( !QSP_packet || !QSP_packet || !QSP_packet->information
      || !QSP_packet->crc || !QSP_packet->serialized_frame || !crcTable )
  {
    for ( ;; )
    {
      // Error!
    }
  }
  crcInit();
  /*The already open usart port is passed through the task parameters*/
  freertos_usart_if xbee_usart = (freertos_usart_if) pvParameters;

  for ( ;; )
  {

    /*--------------------------Receive the frame---------------------
     * The frame length can vary, one has to listen to start and stop flags.*/

    uint32_t nr_bytes_received;
    nr_bytes_received = freertos_usart_serial_read_packet( xbee_usart,  /* The USART port*/
        receive_buffer,                                           /* Pointer to the buffer into which received data is to be copied.*/
        1,                                                        /* The number of bytes to copy. */
        portMAX_DELAY );                                          /* Block time*/

    if ( nr_bytes_received >= 1 )
    {

      for ( int i = 0; i < nr_bytes_received; i++, k++ )
      {
        slip_frame[k] = receive_buffer[i];
        if ( slip_frame[k] == frame_boundary_octet )
        {
          if ( k > 4 ) // Last boundary of frame. Frame minimum length = 4
          {
            slip_frame_length = k;
            k = 0;
            /*handle communication*/
            Slip_Decode(slip_frame, QSP_packet->serialized_frame, slip_frame_length);
            Com_DeserializeQSPFrame(QSP_packet);
            Com_HandleCommunication(QSP_packet);
          }
          else // First boundary of frame.
          {
            k = 0;
            slip_frame[0] = receive_buffer[i];

          }
        }
      }
    }
  }
}



void Com_SendAck( communicaion_packet_t *QSP_packet)
{
  QSP_packet->address = 2;
  QSP_packet->control = ctrl_ack;
  QSP_packet->length = 0;
  Com_SerializeQSPFrame( QSP_packet);
  Com_CommunicationSend( &(QSP_packet->serialized_frame), QSP_packet->serialized_frame_length );
}

void Com_HandleCommunication( communicaion_packet_t *QSP_packet)
{
  // If the packet contains a request to change flight mode, try doing so.
  switch ( QSP_packet->address )
  {
  // notify module.

    break;
  case 1:
    break;
  case 2:
    break;
  default:
    break;
  }


}

void Com_CommunicationSend( const void * const pvItemToQueue, uint8_t bytesToSend )
{
  //TODO add check to make sure sending was successful.
  xQueueSendToBack( xQueue_display, pvItemToQueue, mainDONT_BLOCK );
  xQueueSendToBack( xQueue_display_bytes_to_send, &bytesToSend,
      mainDONT_BLOCK );

}


/*
 *
 * Packet format:
 * [address][control][length][data (0-MAX_DATA_LENGTH bytes)][crc]
 *
 *
 *
 *
 */
uint8_t Com_SerializeQSPFrame( communicaion_packet_t *packet)
{
  packet->serialized_frame[0] = packet->address;
  packet->serialized_frame[1] = packet->control;
  packet->serialized_frame[2] = packet->length;
  int i;
  for ( i = 0; i < packet->length; i++ )
  {
    packet->serialized_frame[3 + i] = packet->information[i];
  }
  //Calculate and append CRC16 checksum.
  uint16_t crc_check;
  crc_check = crcFast( packet->serialized_frame, packet->length + 3 );
  packet->crc[0] = (uint8_t) ((crc_check >> 0) & 0Xff);
  packet->crc[1] = (uint8_t) ((crc_check >> 8) & 0Xff);

  packet->serialized_frame[3 + i++] = packet->crc[0];
  packet->serialized_frame[3 + i] = packet->crc[1];

  uint8_t length = 0; //Slip_encode(); //TODO
  packet->serialized_frame_length = length ;
  return 0;
}

uint8_t Com_DeserializeQSPFrame( communicaion_packet_t *packet)
{


  packet->address = packet->serialized_frame[0];
  packet->control = packet->serialized_frame[1];
  packet->length = packet->serialized_frame[2];

  packet->crc[0] = packet->serialized_frame[3 + packet->length];
  packet->crc[1] = packet->serialized_frame[3 + packet->length + 1];
  for ( int i = 0; i < packet->length; i++ )
  {
    packet->information[i] = packet->serialized_frame[3 + i];
  }
  //Calculate and compare CRC16 checksum.
  uint16_t crc_check;
  uint8_t crc[2];

  crc_check = crcFast( packet->serialized_frame, packet->length + 3 );
  crc[0] = (uint8_t) ((crc_check >> 0) & 0Xff);
  crc[1] = (uint8_t) ((crc_check >> 8) & 0Xff);
  if ( (packet->crc[0] != crc[0]) || (packet->crc[1] != crc[1]) )
  {
    packet->control = ctrl_crc_error;
    return -1;
  }
  return 0;
}



