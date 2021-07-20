/*
* MessagePool.c
*
* Copyright (C) 2021  Martin Lundh
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
#include "MessagePool/inc/messagePool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
typedef struct messageItem messageItem_t;

struct messageItem
{
    SemaphoreHandle_t xSemaphore; // only needs to protect the data, header is fine.
    messageItem_t* nextItem;
    uint8_t* memory;
};

struct messagePool
{
    uint8_t* messages;
    uint32_t numberOfMessages;
    uint32_t itemSize;
    messageItem_t* lastItem;
};


messagePool_t* messagePool_create(uint32_t itemSize, uint32_t itemCount)
{
    messagePool_t* pool = pvPortMalloc(sizeof(messagePool_t));
    uint32_t sizeInPool = (itemSize + sizeof(messageItem_t));
    pool->itemSize = itemSize;
    pool->messages = pvPortMalloc( sizeInPool * itemCount);
    
    for(int i = 0; i < itemCount; i++)
    {
        messageItem_t* item = (messageItem_t*)(&pool->messages[i*sizeInPool]);
        item->xSemaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(item->xSemaphore);
        item->memory = (uint8_t*)(item + 1);
        item->nextItem = (messageItem_t*)(&pool->messages[(i+1)*sizeInPool]);
        if(i == (itemCount-1)) // Last item should be handled separately.
        {
            item->nextItem = (messageItem_t*)(&pool->messages[0]); // close the loop. Last item points to the first.
        }
    }
    pool->lastItem = (messageItem_t*)(&pool->messages[0]); // Start off by pointing to the first area.
    pool->numberOfMessages = itemCount;

    return pool;
}

uint8_t* messagePool_aquire(messagePool_t* pool, uint32_t size)
{
    if(!pool || size > pool->itemSize)
    {
        return NULL;
    }
    uint8_t* result = NULL;
    uint32_t index = 0;
    while(!result && index < pool->numberOfMessages)
    {
        index++;
        pool->lastItem = pool->lastItem->nextItem;
        if(!xSemaphoreTake(pool->lastItem->xSemaphore, 0))
        {
            continue;
        }
        result = pool->lastItem->memory;
    }
    return result;
}

void messagePool_release(messagePool_t* pool, uint8_t* memory)
{
    if(!pool || !memory)
    {
        return;
    }
    messageItem_t* item = (messageItem_t*)(memory - sizeof(messageItem_t));
    if(item->memory != memory)
    {
        return; // TODO error...
    }
    xSemaphoreGive(item->xSemaphore);
}

void messagePool_delete(messagePool_t* pool)
{
    while(pool->lastItem->nextItem)
    {
        vSemaphoreDelete(pool->lastItem->nextItem->xSemaphore);
        messageItem_t* item = pool->lastItem->nextItem;
        pool->lastItem->nextItem = NULL;
        pool->lastItem = item;
    }
    vPortFree(pool->messages);
    vPortFree(pool);
}

