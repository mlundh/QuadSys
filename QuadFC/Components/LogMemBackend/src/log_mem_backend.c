/*
 * log_mem_backend.c
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

#include "FreeRTOS.h"

#include "Log/inc/log_backend.h"
#include "Log/inc/logHandler.h"
#include "HAL/QuadFC/QuadFC_Memory.h"
#include "Utilities/inc/serialization.h"

#define LOG_BACKEND_ITEM_SIZE (12)
#define LOG_BACKEND_SIZE (500000/LOG_BACKEND_ITEM_SIZE)
#define LOG_BACKEND_BASE_ADDR 32000

struct logBackEnd
{
    uint32_t head;
    uint32_t tail;
    uint32_t capacity;
    uint32_t count;
};


LogBackEnd_t* LogBackend_CreateObj()
{
    LogBackEnd_t * obj = pvPortMalloc(sizeof(LogBackEnd_t));
    if(!obj)
    {
        return NULL;
    }
    obj->capacity = LOG_BACKEND_SIZE;
    obj->count = 0;
    obj->head = 0;
    obj->tail = 0;
    return obj;
}

void LogBackend_DeleteObj(LogBackEnd_t* obj)
{
    if(!obj)
    {
        return;
    }
    vPortFree(obj);
}


uint8_t LogBackend_Report(LogBackEnd_t* obj, logEntry_t* entry)
{
    if(obj->count >= obj->capacity)
    {
        // This allows for overwriting of the oldest data.

        //Head is in the correct position, we should adjust tail (push forward one).
        obj->tail++;
        obj->tail%=LOG_BACKEND_SIZE;
        obj->count--;
    }

    // TODO optimize the transfer, allow for multiple entries to be written at the same time. This will minimize SPI overhead.
    uint8_t buffer[LOG_BACKEND_ITEM_SIZE] = {0};
    uint32_t bufferLength = LOG_BACKEND_ITEM_SIZE;

    uint8_t* bufferP = serialize_int32_t(&buffer[0], &bufferLength, &entry->data);
    bufferP = serialize_uint32_t(bufferP, &bufferLength, &entry->id);
    bufferP = serialize_uint32_t(bufferP, &bufferLength, &entry->time);


    if(!Mem_Write(LOG_BACKEND_BASE_ADDR + (obj->head * LOG_BACKEND_ITEM_SIZE), LOG_BACKEND_ITEM_SIZE, buffer,0))
    {
        return 0;
    }

    obj->head++;
    obj->head%=LOG_BACKEND_SIZE;

    obj->count++;
    return 1;
}

uint8_t LogBackend_GetLog(LogBackEnd_t* obj, logEntry_t* entry, uint32_t nrElements, uint32_t* nrLogs)
{
    if(!obj || !entry || !nrLogs)
    {
        return 0;
        //error
    }
    *nrLogs = 0;
    while(obj->count > 0 && nrElements > 0)
    {
        // TODO optimize the transfer, allow for multiple entries to be read at the same time. This will minimize SPI overhead.
        uint8_t buffer[LOG_BACKEND_ITEM_SIZE] = {0};
        if(!Mem_Read(LOG_BACKEND_BASE_ADDR + (obj->tail * LOG_BACKEND_ITEM_SIZE), LOG_BACKEND_ITEM_SIZE, buffer, LOG_BACKEND_ITEM_SIZE))
        {
            return 0;
        }
        uint32_t bufferLength = LOG_BACKEND_ITEM_SIZE;

        uint8_t* bufferP = deserialize_int32_t(&buffer[0], &bufferLength, &entry->data);
        bufferP = deserialize_uint32_t(bufferP, &bufferLength, &entry->id);
        bufferP = deserialize_uint32_t(bufferP, &bufferLength, &entry->time);

        obj->tail++;
        obj->tail%=LOG_BACKEND_SIZE;
        obj->count--;
        nrElements--;
        entry++;
        (*nrLogs)++;
    }
    return 1;
}


