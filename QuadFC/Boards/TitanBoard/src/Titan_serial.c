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


#include "string.h"
#include "QuadFC/QuadFC_Peripherals.h"
#include "HMI/inc/led_control_task.h"



#define RECEIVE_BUFFER_LENGTH (512)


uint8_t QuadFC_SerialInit(int busIndex, QuadFC_SerialOptions_t* opt)
{
  return 0;
}


uint8_t QuadFC_SerialWrite(QuadFC_Serial_t *serial_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  return 0;
}
uint32_t QuadFC_SerialRead(QuadFC_Serial_t *serial_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  return 0;
}






















