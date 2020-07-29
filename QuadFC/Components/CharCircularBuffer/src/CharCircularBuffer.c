/*
 * CharCircularBuffer.c
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
#include "../inc/CharCircularBuffer.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

struct CharCircBuffer
{
  uint8_t*        Buffer;
  uint32_t        capacity;  // maximum number of items in the buffer
  uint32_t        count;     // number of items in the buffer
  uint32_t        head;       // pointer to head
  uint32_t        tail;       // pointer to tail
};

CharCircBuffer_t* CharCircBuff_Create(uint32_t size)
{
  CharCircBuffer_t* obj = pvPortMalloc(sizeof(CharCircBuffer_t));
  if(!obj)
  {
    return NULL;
  }
  obj->Buffer = pvPortMalloc(sizeof(uint8_t)*size);
  if(!obj->Buffer)
  {
    return NULL;
  }
  obj->capacity = size;
  obj->count = 0;
  obj->head = 0;
  obj->tail = 0;
  return obj;
}
void CharCircBuff_Delete(CharCircBuffer_t* obj)
{
  if(obj && obj->Buffer)
  {
    vPortFree(obj->Buffer);
  }
  if(obj)
  {
    vPortFree(obj);
  }
}

uint32_t CharCircBuff_NrElemets(CharCircBuffer_t* obj)
{
  if(!obj)
  {
    return 0;
  }
  return obj->count;
}

uint32_t CharCircBuff_Push(CharCircBuffer_t *obj, uint8_t *buffer, int32_t size)
{
  if(obj->count == obj->capacity)
  {
    return 0;
  }
  int32_t returnValue = 0;

  if (obj->tail <= obj->head)
  {
    int32_t cpySize = (size <= (obj->capacity - obj->head)) ? size : (obj->capacity - obj->head);
    returnValue += cpySize;

    memcpy(&(obj->Buffer[obj->head]), buffer, cpySize);
    buffer += cpySize; // Update the buffer pointer so that we can continue to copy if needed.
    obj->head += cpySize;
    obj->head %= obj->capacity;
    obj->count += cpySize;
    size -= cpySize;
  }
  if (obj->tail > obj->head && size > 0) // we can copy everything in one go.
  {
    int32_t cpySize = size <= obj->tail - obj->head ? size : obj->tail - obj->head;
    returnValue += cpySize;

    memcpy(&(obj->Buffer[obj->head]), buffer, cpySize);
    obj->head += cpySize;
    obj->head %= obj->capacity;
    obj->count += cpySize;
    if (obj->count < 0 || obj->count > obj->capacity)
    {
      obj->count = 0;
    }
  }
  return returnValue;
}

uint32_t CharCircBuff_Pop(CharCircBuffer_t* obj, uint8_t *buffer, int32_t size)
{
  if (obj->count == 0)
  {
    return 0;
  }
  int32_t returnValue = 0;

  if (obj->head <= obj->tail)
  {
    int32_t cpySize = (size <= (obj->capacity - obj->tail)) ? size : (obj->capacity - obj->tail);
    returnValue += cpySize;

    memcpy(buffer, &(obj->Buffer[obj->tail]), cpySize);
    buffer += cpySize; // Update the buffer pointer so that we can continue to copy if needed.
    obj->tail += cpySize;
    obj->tail %= obj->capacity;
    obj->count -= cpySize;
    size  -= cpySize;
    if (obj->count < 0 || obj->count > obj->capacity)
    {
      obj->count = 0;
    }
  }
  if (obj->head > obj->tail && size > 0) // we can copy everything in one go.
  {
    int32_t cpySize = size <= obj->count ? size : obj->count;
    returnValue += cpySize;

    memcpy(buffer, &(obj->Buffer[obj->tail]), cpySize);
    obj->tail += cpySize;
    obj->tail %= obj->capacity;
    obj->count -= cpySize;
    if (obj->count < 0 || obj->count > obj->capacity)
    {
      obj->count = 0;
    }
  }
  return returnValue;
}

