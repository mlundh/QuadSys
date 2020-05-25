/*
 * ArduinoDue_serial.c
 *
 * Copyright (C) 2015 Martin Lundh
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


#include "freertos_uart_serial.h"
#include "string.h"
#include "QuadFC/QuadFC_Peripherals.h"
#include "uart_serial.h"


static  Usart *             usart_inst[]      = {USART0, USART1, USART2, USART3};
static  freertos_usart_if   freertos_usart[]  = {NULL, NULL, NULL, NULL};
static  uint32_t            usart_init[]      = {0, 0, 0, 0};
#define RECEIVE_BUFFER_LENGTH (512)

/**
 * Translation between QuadGS notation and target notation of parity.
 * Please note that translation is done via order in the array. This
 * means that is it important to match the order in the array with the
 * order in the QuadFC_SerialParity_t enum.
 */
static uint32_t Parity[] =
{
    US_MR_PAR_NO,
    US_MR_PAR_ODD,
    US_MR_PAR_EVEN
};

static uint32_t StopBits[] =
{
    US_MR_NBSTOP_1_BIT,
    US_MR_NBSTOP_1_5_BIT,
    US_MR_NBSTOP_2_BIT
};

static uint32_t DataBits[] =
{
    US_MR_CHRL_5_BIT,
    US_MR_CHRL_6_BIT,
    US_MR_CHRL_7_BIT,
    US_MR_CHRL_8_BIT
};

uint8_t QuadFC_SerialInit(int busIndex, QuadFC_SerialOptions_t* opt)
{

  //Return success if the uart is already initialized.
  if(usart_init[busIndex])
  {
    return 1;
  }

  sam_usart_opt_t usart_settings = {
      opt->baudRate,                          /* Speed of the transfer, baud*/
      DataBits[opt->dataBits],                /* 8 bit transfer*/
      Parity[opt->parityType],                /* No parity*/
      StopBits[opt->stopBits],                /* One stop bit*/
      US_MR_CHMODE_NORMAL,                    /* */
      0                                       /* Only used in IrDA mode. */
  };

  freertos_peripheral_options_t driver_options = {
      opt->receiveBuffer,                                   /* The buffer used internally by the USART driver to store incoming characters. */
      opt->bufferLength,                                    /* The size of the buffer provided to the USART driver to store incoming characters. */
      (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 4),   /* The priority used by the USART interrupts. */
      USART_RS232,                                          /* Configure the USART for RS232 operation. */
      (WAIT_TX_COMPLETE | WAIT_RX_COMPLETE)                 /* Wait for a Tx to complete before returning from send functions. */
  };

  freertos_usart_if usart = freertos_usart_serial_init( usart_inst[busIndex], &usart_settings,
      &driver_options );
  if(!usart)
  {
    return 0;
  }

  freertos_usart[busIndex] = usart;
  usart_init[busIndex] = 1;
  return 1;
}


uint8_t QuadFC_SerialWrite(QuadFC_Serial_t *serial_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  if(busIndex > (sizeof(usart_inst)/sizeof(usart_inst[0])))
  {
    return 0;
  }
  if(!usart_init[busIndex])
  {
    return 0;
  }
  TickType_t blockTimeTicks = (blockTimeMs / portTICK_PERIOD_MS);
  status_code_t result = freertos_usart_write_packet( freertos_usart[busIndex],
      serial_data->buffer,
      serial_data->bufferLength,
      blockTimeTicks );

  if ( result != STATUS_OK )
  {
    return 0;
  }
  return 1;
}
uint32_t QuadFC_SerialRead(QuadFC_Serial_t *serial_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  if(busIndex > (sizeof(usart_inst)/sizeof(usart_inst[0])))
  {
    return 0;
  }
  if(!usart_init[busIndex])
  {
    return 0;
  }

  TickType_t blockTimeTicks;
  if(blockTimeMs == portMAX_DELAY)
  {
    blockTimeTicks = portMAX_DELAY;
  }
  else
  {
    blockTimeTicks= (blockTimeMs / portTICK_PERIOD_MS);
  }

  uint32_t nr_bytes_received;
  nr_bytes_received = freertos_usart_serial_read_packet( freertos_usart[busIndex],
      serial_data->buffer,
      serial_data->bufferLength,
      blockTimeTicks );

  return nr_bytes_received;
}






















