/*----------------------------------------------------------------------
 * receiver_task.c
 *
 * QuadFC - Firmware for a quadcopter.
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
 * */

/* ----------------------------------------------------------------------
 * Spectrum satellite receiver task.
 * ----------------------------------------------------------------------
 * Code to parse the serial data received from the spectrum 
 * satellite RC receiver in conjunction with spectrum DX6i transmitter.
 * 
 * Each channel uses two bytes. The data is contained in the
 * 11 least significant bits and channel number is contained
 * in the 4 bits over that.
 *
 * That is one frame looks as: [0 C C C C D D D D D D D D D D D]
 * Where "D" is data and "C" is channel.
 * 
 *
 */
#include "satellite_receiver_task.h"



void satellite_decode_signal( uint8_t data[], receiver_data_t *decoded_data );
static uint8_t receive_buffer[SATELLITE_MESSAGE_LENGTH * 2];

/*void create_satellite_receiver_task(void)
 * Spectrum satellite receiver task. Initialize the USART instance
 * and create the task.
 *
 */
void create_satellite_receiver_task( void )
{

  freertos_usart_if freertos_usart;

  freertos_peripheral_options_t driver_options = { receive_buffer, /* The buffer used internally by the USART driver to store incoming characters. */
  SATELLITE_MESSAGE_LENGTH, /* The size of the buffer provided to the USART driver to store incoming characters. */
  (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3), /* The priority used by the USART interrupts. */
  USART_RS232, /* Configure the USART for RS232 operation. */
  (WAIT_TX_COMPLETE | WAIT_RX_COMPLETE) /* Wait for Tx and Rx to complete before returning from functions. */
  };

  const sam_usart_opt_t usart_settings = {
  BAUD_SATELLITE, /* Speed of the transfer, baud*/
  US_MR_CHRL_8_BIT, /* 8 bit transfer*/
  US_MR_PAR_NO, /* No parity*/
  US_MR_NBSTOP_1_BIT, /* One stop bit*/
  US_MR_CHMODE_NORMAL, /* */
  0 /* Only used in IrDA mode. */
  };

  /* Initialize the USART interface. */
  freertos_usart = freertos_usart_serial_init( SATELITE_USART, &usart_settings,
      &driver_options );
  if ( freertos_usart == NULL )
  {
    for ( ;; )
    {

    }
  }

  /*Create the worker task*/
  xTaskCreate( satellite_receiver_task, /* The task that implements the test. */
  NULL, /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
  500, /* The size of the stack allocated to the task. */
  (void *) freertos_usart, /* The parameter is used to pass the already configured USART port into the task. */
  configMAX_PRIORITIES-2, /* The priority allocated to the task. */
  NULL ); /* Used to store the handle to the created task - in this case the handle is not required. */
}

/* void satellite_receiver_task(void *pvParameters)
 *
 * Task communicating with the satellite receiver. 
 *
 * it is necessary to sync communication with the satellite receiver to
 * ensure that each frame is captured as a whole. A frame consists of 
 * 16 consecutive bytes that should be received without delay. A new 
 * frame should arrive every 11 or 22 ms.

 */
void satellite_receiver_task( void *pvParameters )
{

  uint8_t satellite_receive_buffer[SATELLITE_MESSAGE_LENGTH];
  const portBASE_TYPE satellite_block_time = 75UL / portTICK_RATE_ONE_THIRD_MS;	// One frame each 22 ms. This causes an error message if no valid rc-signal is detected. 75/3 = 25ms between reads
  const portBASE_TYPE satellite_block_time_sync = 10UL
      / portTICK_RATE_ONE_THIRD_MS;		// One frame each 22 ms

  uint8_t bytes_read = 0;
  uint8_t timeout_ok = 0;
  receiver_data_t satellite_data;

  /*The already open usart port is passed through the task parameters*/
  freertos_usart_if spectrum_usart = (freertos_usart_if) pvParameters;

  /*Like most tasks this is realized by a never-ending loop*/

  // TODO add check that the queue is empty
  for ( ;; )
  {

    /*Read data from the satellite receiver, wait longer than the period of the frames.*/
    bytes_read = freertos_usart_serial_read_packet( spectrum_usart,
        satellite_receive_buffer,
        SATELLITE_MESSAGE_LENGTH, satellite_block_time );

    satellite_decode_signal( satellite_receive_buffer, &satellite_data );

    /*If not 16 bytes are read something is wrong */
    if ( (satellite_data.connection_ok == 0)
        || (bytes_read != SATELLITE_MESSAGE_LENGTH) )
    {
      /*error!*/
      satellite_data.connection_ok = 0;
    }
    /*Send the data to the back of the queue*/
    xQueueSendToBack( xQueue_receiver, &satellite_data, mainDONT_BLOCK );

    /*read another 16 bytes, expect a timeout. This synchronizes the receive */
    timeout_ok = freertos_usart_serial_read_packet( spectrum_usart,
        satellite_receive_buffer,
        SATELLITE_MESSAGE_LENGTH * 2, satellite_block_time_sync );
    if ( timeout_ok >= SATELLITE_MESSAGE_LENGTH )
    {
      /*no timeout occurred, error!*/
    }

  }

}

/* -------------------------------------------------------------------
 * Helper functions
 *--------------------------------------------------------------------
 */

/* void satellite_decode_signal(uint8_t data[], receiver_data_t *decoded_data)
 * 
 * Data is written into the struct struct receiver_data *decoded_data, if
 * the field decoded_data->connection_ok is 0 then the decoding was sucsessful,
 * otherwise something is wrong and values in the struct should be discarded. 
 * 
 */
void satellite_decode_signal( uint8_t data[], receiver_data_t *decoded_data )
{
  static int32_t raw_data;
  static int32_t channel;
  static int32_t channel_data;
  static uint8_t check = 0;

  check = 0;
  /*Each channel uses two bytes of information as described in the above comment.*/
  for ( int i = 0; i < SATELLITE_MESSAGE_LENGTH; i += 2 )
  {
    raw_data = ((int32_t) data[i] << 8) | ((int32_t) data[i + 1]);// Combine the two bytes of data that represents one channel.
    channel = (raw_data & SATELLITE_CHANNEL_MASK) >> 11;		// Get the channel.
    channel_data = (raw_data & SATELLITE_DATA_MASK);				// Get the data.

    /* Store the data in the corresponding data field. Each field should be
     * written to exactly once every time the function is called.
     *
     * Sync data arrives first and could be anything. Should always be assigned to default.
     * All other data will be correctly assigned after that. */
    if ( !(check & 0x80) )
    {
      decoded_data->sync = (int32_t) raw_data;
      check |= 0x80;
    }
    else
    {
      switch ( channel )
      {
      case 0:
        if ( !(check & 0x1) ) /* */
        {
          decoded_data->ch0 = (int32_t) channel_data;
          check |= 0x1;	// make sure that each channel is assigned to once and only once.

        }
        break;
      case 1:
        if ( !(check & 0x2) )
        {
          decoded_data->ch1 = (int32_t) channel_data;
          check |= 0x2;

        }
        break;
      case 2:
        if ( !(check & 0x4) )
        {
          decoded_data->ch2 = (int32_t) channel_data;
          check |= 0x4;

        }
        break;
      case 3:
        if ( !(check & 0x8) )
        {
          decoded_data->ch3 = (int32_t) channel_data;
          check |= 0x8;

        }
        break;
      case 4:
        if ( !(check & 0x10) )
        {
          decoded_data->ch4 = (int32_t) channel_data;
          check |= 0x10;

        }
        break;
      case 5:
        if ( !(check & 0x20) )
        {
          decoded_data->ch5 = (int32_t) channel_data;
          check |= 0x20;

        }
        break;
      case 6:
        if ( !(check & 0x40) )
        {
          decoded_data->ch6 = (int32_t) channel_data;
          check |= 0x40;

        }
        break;
      default:
        decoded_data->connection_ok = 1;
        break;
      }
    }
  }
  if ( check != 0xFF ) // If not all bits are set to "1" then something is wrong.
  {
    decoded_data->connection_ok = 0;
  }
  else
  {
    decoded_data->connection_ok = 1;
  }

}

