/*
 * event_circular_buffer.c
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

#include "EventHandler/inc/event_circular_buffer.h"
#include "EventHandler/inc/event_handler.h"

#include <stddef.h>


struct eventCircBuffer
{
  eventData_t     Buffer[BUFFER_SIZE];
  uint8_t         eventBuferIndex;
  size_t          capacity;  // maximum number of items in the buffer
  size_t          count;     // number of items in the buffer
  uint8_t         head;       // pointer to head
  uint8_t         tail;       // pointer to tail
};

eventCircBuffer_t* Event_CreateCBuff()
{
  eventCircBuffer_t* obj = pvPortMalloc(sizeof(eventCircBuffer_t));
  if(!obj)
  {
    return NULL;
  }
  obj->capacity = BUFFER_SIZE;
  obj->count = 0;
  obj->head = 0;
  obj->tail = 0;
  return obj;
}

uint8_t Event_CBuffNrElemets(eventCircBuffer_t* obj)
{
  if(!obj)
  {
    return 0;
  }
  return obj->count;
}

uint8_t Event_CBuffPushBack(eventCircBuffer_t* obj, eventData_t *event)
{
  if(obj->count == obj->capacity)
  {
    return 0;
    //error
  }
  obj->Buffer[obj->head] = *event;
  obj->head++;
  obj->head%=BUFFER_SIZE;

  obj->count++;
  return 1;
}

uint8_t Event_CBuffPopFront(eventCircBuffer_t* obj, eventData_t *event)
{
  if(obj->count == 0)
  {
    return 0;
    //error
  }
  *event = obj->Buffer[obj->tail];
  obj->tail++;
  obj->tail%=BUFFER_SIZE;
  obj->count--;
  return 1;
}
