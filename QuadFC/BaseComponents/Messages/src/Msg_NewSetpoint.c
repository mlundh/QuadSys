/*
 * Msg_NewSetpoint.h
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


#include "../inc/Msg_NewSetpoint.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    state_data_t mSetpoint;
	uint8_t mPriority;
	uint8_t mValidfor;
	
}Msg_NewSetpoint_t;


moduleMsg_t* Msg_NewSetpointCreate(uint32_t destination, uint8_t msgNr
    , state_data_t setpoint, uint8_t priority, uint8_t validFor)
{
    size_t mallocSize = sizeof(moduleMsg_t) + sizeof(Msg_NewSetpoint_t) ;
    moduleMsg_t* msg = pvPortMalloc(mallocSize);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned_e;
        msg->mMsgNr = msgNr;
        msg->type = Msg_NewSetpoint_e;
        msg->mAllocatedSize = mallocSize;

        Msg_NewSetpoint_t* internal_data = (Msg_NewSetpoint_t*)(msg + 1);
        
        internal_data->mSetpoint = setpoint;
        internal_data->mPriority = priority;
        internal_data->mValidfor = validFor;

    }
    return msg;
}

state_data_t Msg_NewSetpointGetSetpoint(moduleMsg_t* msg)
{
    state_data_t value = {0};
    if(msg && (msg->type == Msg_NewSetpoint_e))
    {
        Msg_NewSetpoint_t* internal_data = (Msg_NewSetpoint_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mSetpoint;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_NewSetpointSetSetpoint(moduleMsg_t* msg, state_data_t setpoint)
{
    if(msg && (msg->type == Msg_NewSetpoint_e))
    {
        Msg_NewSetpoint_t* internal_data = (Msg_NewSetpoint_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mSetpoint  = setpoint;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint8_t Msg_NewSetpointGetPriority(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_NewSetpoint_e))
    {
        Msg_NewSetpoint_t* internal_data = (Msg_NewSetpoint_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mPriority;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_NewSetpointSetPriority(moduleMsg_t* msg, uint8_t priority)
{
    if(msg && (msg->type == Msg_NewSetpoint_e))
    {
        Msg_NewSetpoint_t* internal_data = (Msg_NewSetpoint_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mPriority  = priority;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint8_t Msg_NewSetpointGetValidfor(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_NewSetpoint_e))
    {
        Msg_NewSetpoint_t* internal_data = (Msg_NewSetpoint_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mValidfor;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_NewSetpointSetValidfor(moduleMsg_t* msg, uint8_t validFor)
{
    if(msg && (msg->type == Msg_NewSetpoint_e))
    {
        Msg_NewSetpoint_t* internal_data = (Msg_NewSetpoint_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mValidfor  = validFor;
        }
    }
    else
    {
       configASSERT(0);
    }
}

uint8_t* Msg_NewSetpointSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_NewSetpoint_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_NewSetpoint_t* data = (Msg_NewSetpoint_t*)(msg + 1);
        if(data)
        {
            buffer = serialize_state_data_t(buffer, &buffer_size, &data->mSetpoint);
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mPriority);
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mValidfor);

        }
    }
    return buffer;
}

moduleMsg_t* Msg_NewSetpointDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_NewSetpoint_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);
    if(msg)
    {
        msg->mAllocatedSize = size;
        uint8_t* bufferOrig = buffer;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        buffer_size -= buffer - bufferOrig;
        Msg_NewSetpoint_t* data = (Msg_NewSetpoint_t*)(msg + 1);
        if(data)
        {
            buffer = deserialize_state_data_t(buffer, &buffer_size, &data->mSetpoint);
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mPriority);
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mValidfor);

        }
    }
    return msg;
}

