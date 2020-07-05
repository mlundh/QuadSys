/*
 * Utilities_CharCircularBuffer.c
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
#include "../inc/Utilities_CharCircularBuffer.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

struct CharCircBuffer
{
  uint8_t*        Buffer;
  size_t          capacity;  // maximum number of items in the buffer
  size_t          count;     // number of items in the buffer
  uint8_t         head;       // pointer to head
  uint8_t         tail;       // pointer to tail
};

CharCircBuffer_t* Utilities_CBuffCreate(uint32_t size)
{
  CharCircBuffer_t* obj = pvPortMalloc(sizeof(CharCircBuffer_t));
  if(!obj)
  {
    return NULL;
  }
  obj->Buffer = pvPortMalloc(sizeof(uint8_t)*size);
  obj->capacity = size;
  obj->count = 0;
  obj->head = 0;
  obj->tail = 0;
  return obj;
}
void Utilities_CBuffDelete(CharCircBuffer_t* obj)
{
    vPortFree(obj);
}

uint8_t Utilities_CBuffNrElemets(CharCircBuffer_t* obj)
{
  if(!obj)
  {
    return 0;
  }
  return obj->count;
}

uint8_t Utilities_CBuffPush(CharCircBuffer_t* obj, uint8_t item)
{
  taskENTER_CRITICAL();
  int32_t count = obj->count;
  taskEXIT_CRITICAL();

  if(count == obj->capacity)
  {
    return 0;
  }
  obj->Buffer[obj->head] = item;
  obj->head++;
  obj->head%=obj->capacity;
  
  taskENTER_CRITICAL();
  obj->count++;
  taskEXIT_CRITICAL();
  return 1;
}

uint32_t Utilities_CBuffPop(CharCircBuffer_t* obj, uint8_t *buffer, uint32_t size)
{
  taskENTER_CRITICAL();
  int32_t count = obj->count;
  int32_t head = obj->head;
  int32_t tail = obj->tail;
  taskEXIT_CRITICAL();

  if(count == 0)
  {
    return 0;
  }
  int32_t returnValue = 0;

  if(head < tail)
  {
    int32_t cpySize = size <= (obj->capacity - tail) ? size : (obj->capacity - tail);
    returnValue += cpySize;

    memcpy(buffer, &(obj->Buffer[tail]), cpySize);
    tail  += cpySize;
    tail  %= obj->capacity;
    count -= cpySize;
    size  -= cpySize;
  }
  if(head > tail && size > 0) // we can copy everything in one go.
  {
    int32_t cpySize = size <= count ? size : count;
    returnValue += cpySize;

    memcpy(buffer, &(obj->Buffer[tail]), cpySize);
    tail  += cpySize;
    tail  %= obj->capacity;
    count -= cpySize;
  }
  taskENTER_CRITICAL();
  obj->tail = tail;
  obj->count = count;
  taskEXIT_CRITICAL();
  return returnValue;
}

