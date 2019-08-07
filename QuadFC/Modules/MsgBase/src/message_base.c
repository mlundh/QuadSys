/*
 * message_base.c
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

#include "../inc/message_base.h"
#include "Messages/inc/messageTypes.h"
#include "Messages/inc/msgAddr.h"
#include <stddef.h>
#include "FreeRTOS.h"
#include <string.h>


moduleMsg_t* Msg_Create(uint32_t type, uint32_t destination)
{
    size_t size = sizeof(moduleMsg_t);
    moduleMsg_t* msg = pvPortMalloc(size);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned_e;
        msg->mMsgNr = 0;
        msg->type = type;
        msg->mAllocatedSize = sizeof(moduleMsg_t);
    }

    return msg;
}

uint8_t Msg_Delete(moduleMsg_t* msg)
{
    uint8_t result = 0;
    if(msg)
    {
        vPortFree(msg);
        result = 1;
        msg = NULL;
    }
    return result;
}

uint32_t Msg_GetType(moduleMsg_t* msg)
{
    uint32_t type = 0;
    if(msg)
    {
        type = msg->type;
    }
    return type;
}

uint32_t Msg_GetSource(moduleMsg_t* msg)
{
    uint32_t source = 0;
    if(msg)
    {
        source = msg->mSource;
    }
    return source;
}

void Msg_SetSource(moduleMsg_t* msg, uint32_t source)
{
    if(msg)
    {
        msg->mSource = source;
    }
}

uint32_t Msg_GetDestination(moduleMsg_t* msg)
{
    uint32_t dest = 0;
    if(msg)
    {
        dest = msg->mDestination;
    }
    return dest;
}
void Msg_SetDestination(moduleMsg_t* msg, uint32_t desitnation)
{
    if(msg)
    {
        msg->mDestination = desitnation;
    }
}
uint32_t Msg_GetMsgNr(moduleMsg_t* msg)
{
    uint32_t msgNr = 0;
    if(msg)
    {
        msgNr = msg->mMsgNr;
    }
    return msgNr;
}

void Msg_SetMsgNr(moduleMsg_t* msg, uint32_t msgNr)
{
    if(msg)
    {
        msg->mMsgNr = msgNr;
    }
}

uint8_t* Msg_Serialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(!msg || !buffer)
    {
        return 0;
    }
    buffer = serialize_uint32_t(buffer, &buffer_size, &msg->type);
    buffer = serialize_uint32_t(buffer, &buffer_size, &msg->mDestination);
    buffer = serialize_uint32_t(buffer, &buffer_size, &msg->mSource);
    buffer = serialize_uint8_t(buffer, &buffer_size, &msg->mMsgNr);

    return buffer;
}

uint8_t* Msg_DeSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(!msg || !buffer)
    {
        return 0;
    }
    buffer = deserialize_uint32_t(buffer, &buffer_size, &msg->type);
    buffer = deserialize_uint32_t(buffer, &buffer_size, &msg->mDestination);
    buffer = deserialize_uint32_t(buffer, &buffer_size, &msg->mSource);
    buffer = deserialize_uint8_t(buffer, &buffer_size, &msg->mMsgNr);
    return buffer;
}
moduleMsg_t* Msg_Clone(moduleMsg_t* msg)
{
    moduleMsg_t* clone = NULL;
    if(msg)
    {
        clone = pvPortMalloc(msg->mAllocatedSize);
        memcpy(clone, msg, msg->mAllocatedSize);
    }
    return clone;
}

