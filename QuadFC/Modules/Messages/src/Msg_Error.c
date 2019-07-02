/*
 * Msg_Error.h
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


#include "../inc/Msg_Error.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    uint8_t* mError;
	uint32_t mErrorlength;
	uint32_t mErrorbufferlength;
	
}Msg_Error_t;


moduleMsg_t* Msg_ErrorCreate(uint32_t destination, uint8_t msgNr
    , uint32_t Errorbufferlength)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_Error_t)  + (Errorbufferlength);
    moduleMsg_t* msg = pvPortMalloc(size);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned;
        msg->mMsgNr = msgNr;
        msg->type = Msg_Error_e;
        msg->mAllocatedSize = size;

        Msg_Error_t* internal_data = (Msg_Error_t*)(msg + 1);
        
        internal_data->mError = (uint8_t*)(internal_data+1);
        internal_data->mErrorlength = 0;
        internal_data->mErrorbufferlength = Errorbufferlength;

    }
    return msg;
}

uint8_t* Msg_ErrorGetError(moduleMsg_t* msg)
{
    uint8_t* value = {0};
    if(msg && (msg->type == Msg_Error_e))
    {
        Msg_Error_t* internal_data = (Msg_Error_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mError;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_ErrorSetError(moduleMsg_t* msg, uint8_t* error)
{
    if(msg && (msg->type == Msg_Error_e))
    {
        Msg_Error_t* internal_data = (Msg_Error_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mError  = error;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint32_t Msg_ErrorGetErrorlength(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_Error_e))
    {
        Msg_Error_t* internal_data = (Msg_Error_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mErrorlength;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_ErrorSetErrorlength(moduleMsg_t* msg, uint32_t Errorlength)
{
    if(msg && (msg->type == Msg_Error_e))
    {
        Msg_Error_t* internal_data = (Msg_Error_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mErrorlength  = Errorlength;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint32_t Msg_ErrorGetErrorbufferlength(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_Error_e))
    {
        Msg_Error_t* internal_data = (Msg_Error_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mErrorbufferlength;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_ErrorSetErrorbufferlength(moduleMsg_t* msg, uint32_t Errorbufferlength)
{
    if(msg && (msg->type == Msg_Error_e))
    {
        Msg_Error_t* internal_data = (Msg_Error_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mErrorbufferlength  = Errorbufferlength;
        }
    }
    else
    {
       configASSERT(0);
    }
}

uint8_t* Msg_ErrorSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_Error_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_Error_t* data = (Msg_Error_t*)(msg + 1);
        if(data)
        {
            buffer = serialize_string(buffer, &buffer_size, data->mError, data->mErrorlength);

        }
    }
    return buffer;
}

moduleMsg_t* Msg_ErrorDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    moduleMsg_t* msg = pvPortMalloc(buffer_size);

    if(msg)
    {
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        Msg_Error_t* data = (Msg_Error_t*)(msg + 1);
        if(data)
        {
            buffer = deserialize_string(buffer, &buffer_size, data->mError, &data->mErrorlength, data->mErrorbufferlength);

        }
    }
    return msg;
}

