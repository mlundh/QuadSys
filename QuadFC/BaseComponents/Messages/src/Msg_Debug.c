/*
 * Msg_Debug.h
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


#include "../inc/Msg_Debug.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    uint8_t mControl;
	uint8_t* mPayload;
	uint32_t mPayloadlength;
	uint32_t mPayloadbufferlength;
	
}Msg_Debug_t;


moduleMsg_t* Msg_DebugCreatePool(messagePool_t* pool, uint32_t destination, uint8_t msgNr
    , uint8_t control, uint32_t Payloadbufferlength)
{
    size_t mallocSize = sizeof(moduleMsg_t) + sizeof(Msg_Debug_t)  + (Payloadbufferlength);
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
        msg->type = Msg_Debug_e;
        msg->mAllocatedSize = mallocSize;

        Msg_Debug_t* internal_data = (Msg_Debug_t*)(msg + 1);
        
        internal_data->mControl = control;
        internal_data->mPayload = (uint8_t*)(internal_data+1);
        internal_data->mPayloadlength = 0;
        internal_data->mPayloadbufferlength = Payloadbufferlength;

    }
    return msg;
}

uint8_t Msg_DebugGetControl(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_Debug_e))
    {
        Msg_Debug_t* internal_data = (Msg_Debug_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mControl;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_DebugSetControl(moduleMsg_t* msg, uint8_t control)
{
    if(msg && (msg->type == Msg_Debug_e))
    {
        Msg_Debug_t* internal_data = (Msg_Debug_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mControl  = control;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint8_t* Msg_DebugGetPayload(moduleMsg_t* msg)
{
    uint8_t* value = {0};
    if(msg && (msg->type == Msg_Debug_e))
    {
        Msg_Debug_t* internal_data = (Msg_Debug_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mPayload;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_DebugSetPayload(moduleMsg_t* msg, uint8_t* payload)
{
    if(msg && (msg->type == Msg_Debug_e))
    {
        Msg_Debug_t* internal_data = (Msg_Debug_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mPayload  = payload;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint32_t Msg_DebugGetPayloadlength(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_Debug_e))
    {
        Msg_Debug_t* internal_data = (Msg_Debug_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mPayloadlength;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_DebugSetPayloadlength(moduleMsg_t* msg, uint32_t Payloadlength)
{
    if(msg && (msg->type == Msg_Debug_e))
    {
        Msg_Debug_t* internal_data = (Msg_Debug_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mPayloadlength  = Payloadlength;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint32_t Msg_DebugGetPayloadbufferlength(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_Debug_e))
    {
        Msg_Debug_t* internal_data = (Msg_Debug_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mPayloadbufferlength;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_DebugSetPayloadbufferlength(moduleMsg_t* msg, uint32_t Payloadbufferlength)
{
    if(msg && (msg->type == Msg_Debug_e))
    {
        Msg_Debug_t* internal_data = (Msg_Debug_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mPayloadbufferlength  = Payloadbufferlength;
        }
    }
    else
    {
       configASSERT(0);
    }
}

uint8_t* Msg_DebugSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_Debug_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_Debug_t* data = (Msg_Debug_t*)(msg + 1);
        if(data)
        {
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mControl);
            buffer = serialize_string(buffer, &buffer_size, data->mPayload, data->mPayloadlength);

        }
    }
    return buffer;
}

moduleMsg_t* Msg_DebugDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_Debug_t)  + (buffer_size);
    moduleMsg_t* msg = pvPortMalloc(size);
    if(msg)
    {
        msg->mAllocatedSize = size;
        uint8_t* bufferOrig = buffer;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        buffer_size -= buffer - bufferOrig;
        Msg_Debug_t* data = (Msg_Debug_t*)(msg + 1);
        if(data)
        {
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mControl);
            data->mPayloadbufferlength = buffer_size;
            data->mPayload = (uint8_t*)(data+1);
            buffer = deserialize_string(buffer, &buffer_size, data->mPayload, &data->mPayloadlength, data->mPayloadbufferlength);

        }
    }
    return msg;
}

uint32_t Msg_DebugGetMessageSize()
{
    return sizeof(moduleMsg_t) + sizeof(Msg_Debug_t);
}


