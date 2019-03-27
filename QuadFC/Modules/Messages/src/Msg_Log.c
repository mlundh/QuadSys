/*
 * Msg_Log.h
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


#include "../inc/Msg_Log.h"
#include "FreeRTOS.h"

 typedef struct
{
    uint8_t mControl;
	uint8_t* mPayload;
	uint32_t mPayloadlength;
	uint32_t mPayloadbufferlength;
	
}Msg_Log_t;


moduleMsg_t* Msg_LogCreate(uint32_t destination, uint8_t msgNr
    , uint8_t control, uint8_t* payload, uint32_t Payloadlength, uint32_t Payloadbufferlength)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_Log_t);
    moduleMsg_t* msg = pvPortMalloc(size);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned;
        msg->mMsgNr = msgNr;
        msg->type = Msg_Log_e;
        msg->mAllocatedSize = size;

        Msg_Log_t* internal_data = (Msg_Log_t*)(msg + 1);
        
        internal_data->mControl = control;
        internal_data->mPayload = payload;
        internal_data->mPayloadlength = Payloadlength;
        internal_data->mPayloadbufferlength = Payloadbufferlength;

    }
    return msg;
}

uint8_t Msg_LogGetControl(moduleMsg_t* msg)
{
    uint8_t value;
    if(msg && (msg->type == Msg_Log_e))
    {
        Msg_Log_t* internal_data = (Msg_Log_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mControl;
        }
    }
    return value;
}

void Msg_LogSetControl(moduleMsg_t* msg, uint8_t control)
{
    if(msg && (msg->type == Msg_Log_e))
    {
        Msg_Log_t* internal_data = (Msg_Log_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mControl  = control;
        }
    }
}
uint8_t* Msg_LogGetPayload(moduleMsg_t* msg)
{
    uint8_t* value;
    if(msg && (msg->type == Msg_Log_e))
    {
        Msg_Log_t* internal_data = (Msg_Log_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mPayload;
        }
    }
    return value;
}

void Msg_LogSetPayload(moduleMsg_t* msg, uint8_t* payload)
{
    if(msg && (msg->type == Msg_Log_e))
    {
        Msg_Log_t* internal_data = (Msg_Log_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mPayload  = payload;
        }
    }
}
uint32_t Msg_LogGetPayloadlength(moduleMsg_t* msg)
{
    uint32_t value;
    if(msg && (msg->type == Msg_Log_e))
    {
        Msg_Log_t* internal_data = (Msg_Log_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mPayloadlength;
        }
    }
    return value;
}

void Msg_LogSetPayloadlength(moduleMsg_t* msg, uint32_t Payloadlength)
{
    if(msg && (msg->type == Msg_Log_e))
    {
        Msg_Log_t* internal_data = (Msg_Log_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mPayloadlength  = Payloadlength;
        }
    }
}
uint32_t Msg_LogGetPayloadbufferlength(moduleMsg_t* msg)
{
    uint32_t value;
    if(msg && (msg->type == Msg_Log_e))
    {
        Msg_Log_t* internal_data = (Msg_Log_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mPayloadbufferlength;
        }
    }
    return value;
}

void Msg_LogSetPayloadbufferlength(moduleMsg_t* msg, uint32_t Payloadbufferlength)
{
    if(msg && (msg->type == Msg_Log_e))
    {
        Msg_Log_t* internal_data = (Msg_Log_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mPayloadbufferlength  = Payloadbufferlength;
        }
    }
}

uint8_t* Msg_LogSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_Log_e))
    {
        buffer = Msg_Serialize(msg, buffer, &buffer_size);
        Msg_Log_t* data = (Msg_Log_t*)(msg + 1);
        if(data)
        {
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mControl);
            buffer = serialize_string(buffer, &buffer_size, data->mPayload, data->mPayloadlength);

        }
    }
    return buffer;
}

uint8_t* Msg_LogDeserialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_Log_e))
    {
        buffer = Msg_DeSerialize(msg, buffer, &buffer_size);
        Msg_Log_t* data = (Msg_Log_t*)(msg + 1);
        if(data)
        {
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mControl);
            buffer = deserialize_string(buffer, &buffer_size, data->mPayload, &data->mPayloadlength, data->mPayloadbufferlength);

        }
    }
    return buffer;
}
