/*
 * Msg_FcFault.h
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


#include "../inc/Msg_FcFault.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    
}Msg_FcFault_t;


moduleMsg_t* Msg_FcFaultCreatePool(messagePool_t* pool, uint32_t destination, uint8_t msgNr
    )
{
    size_t mallocSize = sizeof(moduleMsg_t) + sizeof(Msg_FcFault_t) ;
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
        msg->type = Msg_FcFault_e;
        msg->mAllocatedSize = mallocSize;

        Msg_FcFault_t* internal_data = (Msg_FcFault_t*)(msg + 1);
        

    }
    return msg;
}


uint8_t* Msg_FcFaultSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_FcFault_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_FcFault_t* data = (Msg_FcFault_t*)(msg + 1);
        if(data)
        {

        }
    }
    return buffer;
}

moduleMsg_t* Msg_FcFaultDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_FcFault_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);
    if(msg)
    {
        msg->mAllocatedSize = size;
        uint8_t* bufferOrig = buffer;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        buffer_size -= buffer - bufferOrig;
        Msg_FcFault_t* data = (Msg_FcFault_t*)(msg + 1);
        if(data)
        {

        }
    }
    return msg;
}

uint32_t Msg_FcFaultGetMessageSize()
{
    return sizeof(moduleMsg_t) + sizeof(Msg_FcFault_t);
}


