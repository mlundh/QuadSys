/*
 * Msg_TestMem.h
 *
 * Copyright (C) 2019 Martin Lundh
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


#include "../inc/Msg_TestMem.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    uint8_t mWrite;
	uint32_t mStartaddr;
	uint8_t mStartnumber;
	uint8_t mSize;
	
}Msg_TestMem_t;


moduleMsg_t* Msg_TestMemCreatePool(messagePool_t* pool, uint32_t destination, uint8_t msgNr
    , uint8_t write, uint32_t startAddr, uint8_t startNumber, uint8_t size)
{
    size_t mallocSize = sizeof(moduleMsg_t) + sizeof(Msg_TestMem_t) ;
    moduleMsg_t* msg = NULL;
    if(pool)
    {
        msg = messagePool_aquire(pool, mallocSize);
        msg->mStatus = 0;
        msg->mStatus |= 0x01;
    }
    else
    {
        msg = pvPortMalloc(mallocSize);
        msg->mStatus = 0;

    }
    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned_e;
        msg->mMsgNr = msgNr;
        msg->mRequireAck = 1;
        msg->type = Msg_TestMem_e;
        msg->mAllocatedSize = mallocSize;

        Msg_TestMem_t* internal_data = (Msg_TestMem_t*)(msg + 1);
        
        internal_data->mWrite = write;
        internal_data->mStartaddr = startAddr;
        internal_data->mStartnumber = startNumber;
        internal_data->mSize = size;

    }
    return msg;
}

uint8_t Msg_TestMemGetWrite(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_TestMem_e))
    {
        Msg_TestMem_t* internal_data = (Msg_TestMem_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mWrite;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_TestMemSetWrite(moduleMsg_t* msg, uint8_t write)
{
    if(msg && (msg->type == Msg_TestMem_e))
    {
        Msg_TestMem_t* internal_data = (Msg_TestMem_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mWrite  = write;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint32_t Msg_TestMemGetStartaddr(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_TestMem_e))
    {
        Msg_TestMem_t* internal_data = (Msg_TestMem_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mStartaddr;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_TestMemSetStartaddr(moduleMsg_t* msg, uint32_t startAddr)
{
    if(msg && (msg->type == Msg_TestMem_e))
    {
        Msg_TestMem_t* internal_data = (Msg_TestMem_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mStartaddr  = startAddr;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint8_t Msg_TestMemGetStartnumber(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_TestMem_e))
    {
        Msg_TestMem_t* internal_data = (Msg_TestMem_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mStartnumber;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_TestMemSetStartnumber(moduleMsg_t* msg, uint8_t startNumber)
{
    if(msg && (msg->type == Msg_TestMem_e))
    {
        Msg_TestMem_t* internal_data = (Msg_TestMem_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mStartnumber  = startNumber;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint8_t Msg_TestMemGetSize(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_TestMem_e))
    {
        Msg_TestMem_t* internal_data = (Msg_TestMem_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mSize;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_TestMemSetSize(moduleMsg_t* msg, uint8_t size)
{
    if(msg && (msg->type == Msg_TestMem_e))
    {
        Msg_TestMem_t* internal_data = (Msg_TestMem_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mSize  = size;
        }
    }
    else
    {
       configASSERT(0);
    }
}

uint8_t* Msg_TestMemSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_TestMem_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_TestMem_t* data = (Msg_TestMem_t*)(msg + 1);
        if(data)
        {
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mWrite);
            buffer = serialize_uint32_t(buffer, &buffer_size, &data->mStartaddr);
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mStartnumber);
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mSize);

        }
    }
    return buffer;
}

moduleMsg_t* Msg_TestMemDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_TestMem_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);
    if(msg)
    {
        msg->mAllocatedSize = size;
        uint8_t* bufferOrig = buffer;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        buffer_size -= buffer - bufferOrig;
        Msg_TestMem_t* data = (Msg_TestMem_t*)(msg + 1);
        if(data)
        {
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mWrite);
            buffer = deserialize_uint32_t(buffer, &buffer_size, &data->mStartaddr);
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mStartnumber);
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mSize);

        }
    }
    return msg;
}

uint32_t Msg_TestMemGetMessageSize()
{
    return sizeof(moduleMsg_t) + sizeof(Msg_TestMem_t);
}


