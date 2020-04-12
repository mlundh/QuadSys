/*
 * memory.c
 *
 * Copyright (C) 2015 martin
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

#include "FreeRTOS.h"
#include "QuadFC/QuadFC_Memory.h"
#include "MB85RC_i2c_fram/inc/MB85RC_i2c_memory.h"
#include "HAL/QuadFC/QuadFC_Peripherals.h"

#define MB85RC_DEFAULT_ADDRESS        (0x50) /* 1010 + A2 + A1 + A0 = 0x50 default */
#define MB85RC_SLAVE_ID               (0xF8)
#define MB85RC_BUSS                   (0x0)
#define MB85RC_BLOCK_TIME             (50UL / portTICK_PERIOD_MS)

uint8_t MB85RC_MemRead(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size)
{
  QuadFC_I2C_t i2c_data;
  i2c_data.buffer = buffer;
  i2c_data.bufferLength = size;
  i2c_data.internalAddrLength = 2;
  i2c_data.internalAddr[0] = (addr >> 8) & 0xff;
  i2c_data.internalAddr[1] = (addr) & 0xff;
  i2c_data.slaveAddress = MB85RC_DEFAULT_ADDRESS;
  if(!QuadFC_i2cRead(&i2c_data, MB85RC_BUSS, MB85RC_BLOCK_TIME))
  {
    return 0;
  }
  return 1;
}

uint8_t MB85RC_MemWrite(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size)
{
  QuadFC_I2C_t i2c_data;
  i2c_data.buffer = buffer;
  i2c_data.bufferLength = size;
  i2c_data.internalAddrLength = 2;
  i2c_data.internalAddr[0] = (addr >> 8) & 0xff;
  i2c_data.internalAddr[1] = (addr) & 0xff;
  i2c_data.slaveAddress = MB85RC_DEFAULT_ADDRESS;
  if(!QuadFC_i2cWrite(&i2c_data, MB85RC_BUSS, MB85RC_BLOCK_TIME))
  {
    return 0;
  }
  return 1;
}
/*
uint8_t MB85RC_getDeviceID(uint16_t *manufacturerID, uint16_t *productID)
{
   uint8_t buffer[3];
   QuadFC_I2C i2c_data;
   i2c_data.buffer = buffer;
   i2c_data.bufferLength = 3;
   i2c_data.internalAddrLength = 2;
   i2c_data.internalAddr[0] = (MB85RC_SLAVE_ID >> 8) & 0xff;
   i2c_data.internalAddr[1] = (MB85RC_SLAVE_ID) & 0xff;
   if(!QuadFC_i2cWrite(i2c_data, MB85RC_BUSS, MB85RC_BLOCK_TIME))
   {
     return 1;
   }

   // Shift values to separate manufacturer and product IDs
   // See p.10 of http://www.fujitsu.com/downloads/MICRO/fsa/pdf/products/memory/fram/MB85RC256V-DS501-00017-3v0-E.pdf
   *manufacturerID = (buffer[0] << 4) + (buffer[1]  >> 4);
   *productID = ((buffer[1] & 0x0F) << 8) + buffer[2];
   return 1;
}
*/
