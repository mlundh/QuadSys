/*
 * Msg_LogNameReq.h
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


#include "../inc/Msg_LogNameReq.h"
#include "FreeRTOS.h"

 typedef struct
{
    QueueHandle_t mData;
	
}Msg_LogNameReq_t;


moduleMsg_t* Msg_LogNameReqCreate(uint32_t destination, uint8_t msgNr
    , QueueHandle_t data)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_LogNameReq_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned;
        msg->mMsgNr = msgNr;
        msg->type = Msg_LogNameReq_e;
        msg->mAllocatedSize = size;

        Msg_LogNameReq_t* internal_data = (Msg_LogNameReq_t*)(msg + 1);
        
        internal_data->mData = data;

    }
    return msg;
}

QueueHandle_t Msg_LogNameReqGetData(moduleMsg_t* msg)
{
    QueueHandle_t value = {0};
    if(msg && (msg->type == Msg_LogNameReq_e))
    {
        Msg_LogNameReq_t* internal_data = (Msg_LogNameReq_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mData;
        }
    }
    return value;
}

void Msg_LogNameReqSetData(moduleMsg_t* msg, QueueHandle_t data)
{
    if(msg && (msg->type == Msg_LogNameReq_e))
    {
        Msg_LogNameReq_t* internal_data = (Msg_LogNameReq_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mData  = data;
        }
    }
}

uint8_t* Msg_LogNameReqSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_LogNameReq_e))
    {
        buffer = Msg_Serialize(msg, buffer, &buffer_size);
        Msg_LogNameReq_t* data = (Msg_LogNameReq_t*)(msg + 1);
        if(data)
        {

        }
    }
    return buffer;
}

uint8_t* Msg_LogNameReqDeserialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_LogNameReq_e))
    {
        buffer = Msg_DeSerialize(msg, buffer, &buffer_size);
        Msg_LogNameReq_t* data = (Msg_LogNameReq_t*)(msg + 1);
        if(data)
        {

        }
    }
    return buffer;
}