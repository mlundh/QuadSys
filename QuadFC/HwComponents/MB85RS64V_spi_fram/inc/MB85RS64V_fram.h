/*
 * MB85RS64V_fram.h
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
#ifndef PORTLAYER_MB85RS64V_SPI_FRAM_INC_MB85RS64V_FRAM_H_
#define PORTLAYER_MB85RS64V_SPI_FRAM_INC_MB85RS64V_FRAM_H_
#define MB85RS64V_SIZE 8000

#include "stdint.h"
#include "stddef.h"
typedef enum opcodes
{
  OPCODE_WREN   = 0b0110,     //!< Write Enable Latch
  OPCODE_WRDI   = 0b0100,     //!< Reset Write Enable Latch */
  OPCODE_RDSR   = 0b0101,     //!< Read Status Register */
  OPCODE_WRSR   = 0b0001,     //!< Write Status Register */
  OPCODE_READ   = 0b0011,     //!< Read Memory */
  OPCODE_WRITE  = 0b0010,     //!< Write Memory */
  OPCODE_RDID   = 0b10011111  //!< Read Device ID */
} opcodes_t;

uint8_t MB85RS64V_MemRead(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size);
uint8_t MB85RS64V_MemWrite(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size);
uint8_t MB85RS64V_WriteEnable(uint8_t enable);

#endif /* PORTLAYER_MB85RS64V_SPI_FRAM_INC_MB85RS64V_FRAM_H_ */
