/*
 * event_circular_buffer.h
 *
 * Copyright (C) 2016 Martin Lundh
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
#ifndef MODULES_EVENTHANDLER_INC_EVENT_CIRCULAR_BUFFER_H_
#define MODULES_EVENTHANDLER_INC_EVENT_CIRCULAR_BUFFER_H_

#include "Utilities/inc/common_types.h"
#include <stdint.h>

typedef struct eventCircBuffer eventCircBuffer_t;
#define BUFFER_SIZE (16)

/**
 * Create the circular buffer.
 * @return  Pointer to the initialized buffer.
 */
eventCircBuffer_t* Event_CreateCBuff();

/**
 * Get the number of buffered events.
 * @param obj     Current buffer.
 * @return        Number of events in the buffer.
 */
uint8_t Event_CBuffNrElemets(eventCircBuffer_t* obj);

/**
 * Push back onto the buffer. Will return false if there is no space
 * left in the buffer. Data is copied.
 * @param obj     Current buffer.
 * @param event   Event to push back.
 * @return        1 if ok, 0 if no space left in buffer.
 */
uint8_t Event_CBuffPushBack(eventCircBuffer_t* obj, eventData_t *event);

/**
 * Pop the front element of the buffer. Returns false if there is no
 * elements in the buffer.
 * @param obj     Current buffer.
 * @param event   Event to copy the data into.
 * @return        1 if ok, 0 if no data was availible in the buffer.
 */
uint8_t Event_CBuffPopFront(eventCircBuffer_t* obj, eventData_t *event);

#endif /* MODULES_EVENTHANDLER_INC_EVENT_CIRCULAR_BUFFER_H_ */
