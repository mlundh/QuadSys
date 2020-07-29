/*
 * CharCircularBuffer.h
 *
 * Copyright (C) 2020 Martin Lundh
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

#ifndef _CHARCIRCULARBUFFER_H_
#define _CHARCIRCULARBUFFER_H_


#include <stdint.h>

typedef struct CharCircBuffer CharCircBuffer_t;

/**
 * Create the circular buffer. 
 * @return  Pointer to the initialized buffer.
 */
CharCircBuffer_t* CharCircBuff_Create(uint32_t size);

/**
 * Delete the buffer.
 * @param obj
 */
void CharCircBuff_Delete(CharCircBuffer_t* obj);

/**
 * Get the number of buffered items.
 * @param obj     Current buffer.
 * @return        Number of items in the buffer.
 */
uint32_t CharCircBuff_NrElemets(CharCircBuffer_t* obj);

/**
 * Push onto the buffer. Will return the number of items pushed.
 * @param obj     Current buffer.
 * @param buffer  Buffer that holds the data that should be written.
 * @param size    Size of the buffer.
 * @return        number of bytes copied to the buffer.
 */
uint32_t CharCircBuff_Push(CharCircBuffer_t *obj, uint8_t *buffer, int32_t size);

/**
 * Pop the oldest element of the buffer. Returns false if there is no
 * elements in the buffer.
 * @param obj     Current buffer.
 * @param buffer  Buffer the data should be written to.
 * @param size    Size of the buffer.
 * @return        number of bytes copied to the buffer.
 */
uint32_t CharCircBuff_Pop(CharCircBuffer_t* obj, uint8_t *buffer, int32_t size);

#endif // _CHARCIRCULARBUFFER_H_