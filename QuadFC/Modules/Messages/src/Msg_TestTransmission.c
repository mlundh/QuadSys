/*
 * Msg_TestTransmission.h
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


#include "../inc/Msg_TestTransmission.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    uint32_t mTest;
	uint8_t* mPayload;
	uint32_t mPayloadlength;
	uint32_t mPayloadbufferlength;
	
}Msg_TestTransmission_t;


moduleMsg_t* Msg_TestTransmissionCreate(uint32_t destination, uint8_t msgNr
    , uint32_t test, uint32_t Payloadbufferlength)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_TestTransmission_t)  + (Payloadbufferlength);
    moduleMsg_t* msg = pvPortMalloc(size);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned_e;
        msg->mMsgNr = msgNr;
        msg->type = Msg_TestTransmission_e;
        msg->mAllocatedSize = size;

        Msg_TestTransmission_t* internal_data = (Msg_TestTransmission_t*)(msg + 1);
        
        internal_data->mTest = test;
        internal_data->mPayload = (uint8_t*)(internal_data+1);
        internal_data->mPayloadlength = 0;
        internal_data->mPayloadbufferlength = Payloadbufferlength;

    }
    return msg;
}

uint32_t Msg_TestTransmissionGetTest(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_TestTransmission_e))
    {
        Msg_TestTransmission_t* internal_data = (Msg_TestTransmission_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mTest;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_TestTransmissionSetTest(moduleMsg_t* msg, uint32_t test)
{
    if(msg && (msg->type == Msg_TestTransmission_e))
    {
        Msg_TestTransmission_t* internal_data = (Msg_TestTransmission_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mTest  = test;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint8_t* Msg_TestTransmissionGetPayload(moduleMsg_t* msg)
{
    uint8_t* value = {0};
    if(msg && (msg->type == Msg_TestTransmission_e))
    {
        Msg_TestTransmission_t* internal_data = (Msg_TestTransmission_t*)(msg + 1);
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

void Msg_TestTransmissionSetPayload(moduleMsg_t* msg, uint8_t* payload)
{
    if(msg && (msg->type == Msg_TestTransmission_e))
    {
        Msg_TestTransmission_t* internal_data = (Msg_TestTransmission_t*)(msg + 1);
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
uint32_t Msg_TestTransmissionGetPayloadlength(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_TestTransmission_e))
    {
        Msg_TestTransmission_t* internal_data = (Msg_TestTransmission_t*)(msg + 1);
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

void Msg_TestTransmissionSetPayloadlength(moduleMsg_t* msg, uint32_t Payloadlength)
{
    if(msg && (msg->type == Msg_TestTransmission_e))
    {
        Msg_TestTransmission_t* internal_data = (Msg_TestTransmission_t*)(msg + 1);
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
uint32_t Msg_TestTransmissionGetPayloadbufferlength(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_TestTransmission_e))
    {
        Msg_TestTransmission_t* internal_data = (Msg_TestTransmission_t*)(msg + 1);
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

void Msg_TestTransmissionSetPayloadbufferlength(moduleMsg_t* msg, uint32_t Payloadbufferlength)
{
    if(msg && (msg->type == Msg_TestTransmission_e))
    {
        Msg_TestTransmission_t* internal_data = (Msg_TestTransmission_t*)(msg + 1);
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

uint8_t* Msg_TestTransmissionSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_TestTransmission_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_TestTransmission_t* data = (Msg_TestTransmission_t*)(msg + 1);
        if(data)
        {
            buffer = serialize_uint32_t(buffer, &buffer_size, &data->mTest);
            buffer = serialize_string(buffer, &buffer_size, data->mPayload, data->mPayloadlength);

        }
    }
    return buffer;
}

moduleMsg_t* Msg_TestTransmissionDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    moduleMsg_t* msg = pvPortMalloc(buffer_size);
    if(msg)
    {
        msg->mAllocatedSize = buffer_size;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        Msg_TestTransmission_t* data = (Msg_TestTransmission_t*)(msg + 1);
        if(data)
        {
            buffer = deserialize_uint32_t(buffer, &buffer_size, &data->mTest);
            buffer = deserialize_string(buffer, &buffer_size, data->mPayload, &data->mPayloadlength, data->mPayloadbufferlength);

        }
    }
    return msg;
}

