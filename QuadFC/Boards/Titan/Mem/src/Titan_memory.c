/*
 * ArduinoDue_memory.c
 *
 * Copyright (C) 2017 Martin Lundh
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

#include "HAL/QuadFC/QuadFC_Memory.h"
#include "HwComponents/CY15B104Q_SX_spi_fram/inc/CY15B104Q-SX_fram.h"

uint8_t Mem_Init()
{
  CY15B104Q_SX_init();
  return 1;
}


uint8_t Mem_Read(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size)
{
  if(addr < CY15B104Q_SX_SIZE) // SPI memory
  {
    return CY15B104Q_SX_MemRead(addr, size, buffer, buffer_size);
  }
  else return 0;
}

uint8_t Mem_Write(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size)
{
  if(addr < CY15B104Q_SX_SIZE) // SPI memory
  {
    return CY15B104Q_SX_MemWrite(addr, size, buffer, buffer_size);
  }
  else return 0;
}


uint8_t Mem_ReadWriteRaw(uint8_t *RxBuffer, uint8_t *TxBuffer, uint32_t buffer_size)
{
  return CY15B104Q_SX_MemReadWriteRaw(RxBuffer, TxBuffer, buffer_size);
}
