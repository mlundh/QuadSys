/*
 * Utilities_CharCircularBuffer.h
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

#ifndef _UTILITIES_CHARCIRCULARBUFFER_H_
#define _UTILITIES_CHARCIRCULARBUFFER_H_


#include <stdint.h>

typedef struct CharCircBuffer CharCircBuffer_t;

/**
 * Create the circular buffer. This buffer is designed to push data in an ISR
 * and pop in application code. 
 * @return  Pointer to the initialized buffer.
 */
CharCircBuffer_t* Utilities_CBuffCreate(uint32_t size);

/**
 * Delete the event buffer.
 * @param obj
 */
void Utilities_CBuffDelete(CharCircBuffer_t* obj);

/**
 * Get the number of buffered events.
 * @param obj     Current buffer.
 * @return        Number of events in the buffer.
 */
uint8_t Utilities_CBuffNrElemets(CharCircBuffer_t* obj);

/**
 * Push onto the buffer. Will return 0 if there is no room
 * in the buffer.
 * @param obj     Current buffer.
 * @param item    item to push back.
 * @return        1 if ok, 0 if the buffer is full.
 */
uint8_t Utilities_CBuffPush(CharCircBuffer_t* obj, uint8_t item);

/**
 * Pop the oldest element of the buffer. Returns false if there is no
 * elements in the buffer.
 * @param obj     Current buffer.
 * @param msg     Pointer to destination.
 * @return        number of bytes copied from the buffer.
 */
uint32_t Utilities_CBuffPop(CharCircBuffer_t* obj, uint8_t *buffer, uint32_t size);

#endif // _UTILITIES_CHARCIRCULARBUFFER_H_