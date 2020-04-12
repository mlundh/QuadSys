/*
 * ArduinoDueTest_serial.c
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


#include "string.h"
#include "QuadFC/QuadFC_Peripherals.h"
#include "status_codes.h"
#include "uart_serial.h"



static  uint32_t			test_usart_init   = 0;
#define RECEIVE_BUFFER_LENGTH (512)



uint8_t Test_SerialInit()
{

  //Return success if the uart is already initialized.
  if(test_usart_init)
  {
    return 1;
  }

  static usart_serial_options_t usart_options = {
      .baudrate = 57600,
      .charlength = US_MR_CHRL_8_BIT,
      .paritytype = US_MR_PAR_NO,
      .stopbits = false
  };

  usart_serial_init(USART3, &usart_options);

  test_usart_init = 1;
  return 1;
}


uint8_t Test_SerialWrite(QuadFC_Serial_t *serial_data)
{

  if(!test_usart_init)
  {
    return 0;
  }

  usart_serial_write_packet(USART3, serial_data->buffer, serial_data->bufferLength);

  return 1;
}



















