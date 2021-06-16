/*
 * Msg_TestMemReg.h
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


#include "../inc/Msg_TestMemReg.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    uint8_t mDeviceid;
	uint8_t mRdsr;
	
}Msg_TestMemReg_t;


moduleMsg_t* Msg_TestMemRegCreate(uint32_t destination, uint8_t msgNr
    , uint8_t deviceId, uint8_t rdsr)
{
    size_t mallocSize = sizeof(moduleMsg_t) + sizeof(Msg_TestMemReg_t) ;
    moduleMsg_t* msg = pvPortMalloc(mallocSize);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned_e;
        msg->mMsgNr = msgNr;
        msg->mRequireAck = 1;
        msg->type = Msg_TestMemReg_e;
        msg->mAllocatedSize = mallocSize;

        Msg_TestMemReg_t* internal_data = (Msg_TestMemReg_t*)(msg + 1);
        
        internal_data->mDeviceid = deviceId;
        internal_data->mRdsr = rdsr;

    }
    return msg;
}

uint8_t Msg_TestMemRegGetDeviceid(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_TestMemReg_e))
    {
        Msg_TestMemReg_t* internal_data = (Msg_TestMemReg_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mDeviceid;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_TestMemRegSetDeviceid(moduleMsg_t* msg, uint8_t deviceId)
{
    if(msg && (msg->type == Msg_TestMemReg_e))
    {
        Msg_TestMemReg_t* internal_data = (Msg_TestMemReg_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mDeviceid  = deviceId;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint8_t Msg_TestMemRegGetRdsr(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_TestMemReg_e))
    {
        Msg_TestMemReg_t* internal_data = (Msg_TestMemReg_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mRdsr;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_TestMemRegSetRdsr(moduleMsg_t* msg, uint8_t rdsr)
{
    if(msg && (msg->type == Msg_TestMemReg_e))
    {
        Msg_TestMemReg_t* internal_data = (Msg_TestMemReg_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mRdsr  = rdsr;
        }
    }
    else
    {
       configASSERT(0);
    }
}

uint8_t* Msg_TestMemRegSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_TestMemReg_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_TestMemReg_t* data = (Msg_TestMemReg_t*)(msg + 1);
        if(data)
        {
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mDeviceid);
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mRdsr);

        }
    }
    return buffer;
}

moduleMsg_t* Msg_TestMemRegDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_TestMemReg_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);
    if(msg)
    {
        msg->mAllocatedSize = size;
        uint8_t* bufferOrig = buffer;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        buffer_size -= buffer - bufferOrig;
        Msg_TestMemReg_t* data = (Msg_TestMemReg_t*)(msg + 1);
        if(data)
        {
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mDeviceid);
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mRdsr);

        }
    }
    return msg;
}

