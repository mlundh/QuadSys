/*
 * range_meter.c
 *
 * Copyright (C) 2014 martin
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

/*
 * Ranger hardware: HRLV-MaxSonar MB1003 by Maxbotix.
 *
 * Output: TTL logic (by jumper on back of unit) ASCII
 * Output format: 6 bytes, first 'R' followed by range data D in
 * ASCII format followed by a carridge return '\r' (ASCII 13).
 *
 * */

#include "PortLayer/Sensors/inc/range_meter.h"
#include "freertos_uart_serial.h"
#include "Utilities/inc/string_utils.h"
#include "Utilities/inc/globals.h"

int32_t moving_average( int32_t *buffer, int32_t new, uint8_t *i,
    int32_t length );
static uint8_t receive_buffer[RANGE_METER_MESSAGE_LENGTH * 2];

/*void create_range_meter_task(void)
 * Spectrum satellite receiver task. Initialize the USART instance
 * and create the task.
 *
 */

void create_range_meter_task( void )
{

  freertos_usart_if freertos_usart;

  freertos_peripheral_options_t driver_options = { receive_buffer,      /* The buffer used internally by the USART driver to store incoming characters. */
      RANGE_METER_MESSAGE_LENGTH,                                       /* The size of the buffer provided to the USART driver to store incoming characters. */
      (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 3),               /* The priority used by the USART interrupts. */
      USART_RS232,                                                      /* Configure the USART for RS232 operation. */
      (WAIT_TX_COMPLETE | WAIT_RX_COMPLETE)                             /* Wait for Tx and Rx to complete before returning from functions. */
  };

  const sam_usart_opt_t usart_settings = {
      BAUD_RANGE_METER,                         /* Speed of the transfer, baud*/
      US_MR_CHRL_8_BIT,                         /* 8 bit transfer*/
      US_MR_PAR_NO,                             /* No parity*/
      US_MR_NBSTOP_1_BIT,                       /* One stop bit*/
      US_MR_CHMODE_NORMAL,                      /* */
      0                                         /* Only used in IrDA mode. */
  };

  /* Initialize the USART interface. */
  freertos_usart = freertos_usart_serial_init( RANGE_USART, &usart_settings,
      &driver_options );
  if ( freertos_usart == NULL )
  {
    for ( ;; )
    {
      /*TODO handle this error!*/
    }
  }

  /*Create the worker task*/
  xTaskCreate( range_meter_task,
      "Range",
      500,
      (void * ) freertos_usart,
      configMAX_PRIORITIES-2,
      NULL );
}

/* void range_meter_task(void *pvParameters)
 *
 * Task communicating with the range meter receiver.
 *
 * Sync is achieved by listening for an ascii 'R'.
 */
void range_meter_task( void *pvParameters )
{
  const portBASE_TYPE range_block_time = 200UL / portTICK_PERIOD_MS;

  uint8_t ranger_receive_buffer[RANGE_METER_MESSAGE_LENGTH];
  uint8_t bytes_read = 0;
  int32_t distance = 0;
  int32_t distance_calc[5] = { 0 };
  int32_t new_sample = 0;
  uint8_t index = 0;

  /*The already open usart port is passed through the task parameters*/
  freertos_usart_if range_usart = (freertos_usart_if) pvParameters;

  for ( ;; )
  {

    /*Read data from the ranger, wait longer than the period of the frames.*/
    bytes_read = freertos_usart_serial_read_packet( range_usart,
        ranger_receive_buffer,
        RANGE_METER_MESSAGE_LENGTH, range_block_time );

    if ( RANGE_METER_MESSAGE_LENGTH != bytes_read )
    {
      distance = -5;
      xQueueSendToBack( xQueue_ranger, &distance, mainDONT_BLOCK );
      //error!
    }
    if ( ranger_receive_buffer[0] == 'R' )
    {
      new_sample = my_atoi( &ranger_receive_buffer[1], 4 );
      distance = moving_average( distance_calc, new_sample, &index, 5 );
      xQueueSendToBack( xQueue_ranger, &new_sample, mainDONT_BLOCK );
    }
    else // out of sync, resync!
    {
      while ( ranger_receive_buffer[0] != 13 ) // '\r' is ascii 13 and the end of the message.
      {
        bytes_read = freertos_usart_serial_read_packet( range_usart,
            ranger_receive_buffer, 1, range_block_time );
        if ( bytes_read != 1 )
        {
          /*TODO error!*/
        }
      }
    }
  }
}
/*Simple moving average filter.  */
int32_t moving_average( int32_t *buffer, int32_t new, uint8_t *i,
    int32_t length )
{
  if ( (i < 0) || (*i > length) || (!buffer) )
  {
    // Error, return -1
    return -1;
  }
  buffer[*i] = new / length;
  *i = *i + 1;
  if ( *i >= length )
  {
    *i = 0;
  }
  int32_t res = 0;
  for ( int k = 0; k < length; k++ )
  {
    res += buffer[k];
  }
  return res;
}
