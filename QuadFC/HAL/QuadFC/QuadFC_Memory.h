/*
 * memory.h
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
#ifndef PORTLAYER_MEMORY_INC_QUAD_FC_MEMORY_H_
#define PORTLAYER_MEMORY_INC_QUAD_FC_MEMORY_H_

#include "stdint.h"

/**
 * Read from memory.
 * @param addr          Address in memory to read from.
 * @param size          Size to read.
 * @param buffer        Buffer to read into.
 * @param buffer_size   Buffer size.
 * @return              1 if success, 0 otherwise.
 */
uint8_t Mem_Read(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size);


/**
 * Write to memory.
 * @param addr          Address in memory to write to.
 * @param size          Size to Write.
 * @param buffer        Buffer to Write from.
 * @param buffer_size   Buffer size.
 * @return              1 if success, 0 otherwise.
 */
uint8_t Mem_Write(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size);

#endif /* PORTLAYER_MEMORY_INC_MEMORY_H_ */
