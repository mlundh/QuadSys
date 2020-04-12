/*
 * MB85RC_i2c_memory.h
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
#ifndef PORTLAYER_MB85RC_I2C_FRAM_INC_MB85RC_I2C_MEMORY_H_
#define PORTLAYER_MB85RC_I2C_FRAM_INC_MB85RC_I2C_MEMORY_H_

#define MB85RC_SIZE 32000
#include "stdint.h"
#include "stddef.h"

uint8_t MB85RC_MemRead(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size);
uint8_t MB85RC_MemWrite(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size);



#endif /* PORTLAYER_MB85RC_I2C_FRAM_INC_MB85RC_I2C_MEMORY_H_ */
