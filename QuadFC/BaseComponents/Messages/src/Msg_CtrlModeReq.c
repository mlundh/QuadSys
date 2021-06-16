/*
 * Msg_CtrlModeReq.h
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


#include "../inc/Msg_CtrlModeReq.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    uint8_t mMode;
	
}Msg_CtrlModeReq_t;


moduleMsg_t* Msg_CtrlModeReqCreate(uint32_t destination, uint8_t msgNr
    , uint8_t mode)
{
    size_t mallocSize = sizeof(moduleMsg_t) + sizeof(Msg_CtrlModeReq_t) ;
    moduleMsg_t* msg = pvPortMalloc(mallocSize);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned_e;
        msg->mMsgNr = msgNr;
        msg->mRequireAck = 1;
        msg->type = Msg_CtrlModeReq_e;
        msg->mAllocatedSize = mallocSize;

        Msg_CtrlModeReq_t* internal_data = (Msg_CtrlModeReq_t*)(msg + 1);
        
        internal_data->mMode = mode;

    }
    return msg;
}

uint8_t Msg_CtrlModeReqGetMode(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_CtrlModeReq_e))
    {
        Msg_CtrlModeReq_t* internal_data = (Msg_CtrlModeReq_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mMode;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_CtrlModeReqSetMode(moduleMsg_t* msg, uint8_t mode)
{
    if(msg && (msg->type == Msg_CtrlModeReq_e))
    {
        Msg_CtrlModeReq_t* internal_data = (Msg_CtrlModeReq_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mMode  = mode;
        }
    }
    else
    {
       configASSERT(0);
    }
}

uint8_t* Msg_CtrlModeReqSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_CtrlModeReq_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_CtrlModeReq_t* data = (Msg_CtrlModeReq_t*)(msg + 1);
        if(data)
        {
            buffer = serialize_uint8_t(buffer, &buffer_size, &data->mMode);

        }
    }
    return buffer;
}

moduleMsg_t* Msg_CtrlModeReqDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_CtrlModeReq_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);
    if(msg)
    {
        msg->mAllocatedSize = size;
        uint8_t* bufferOrig = buffer;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        buffer_size -= buffer - bufferOrig;
        Msg_CtrlModeReq_t* data = (Msg_CtrlModeReq_t*)(msg + 1);
        if(data)
        {
            buffer = deserialize_uint8_t(buffer, &buffer_size, &data->mMode);

        }
    }
    return msg;
}

