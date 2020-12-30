/*
 * Msg_ValidSp.h
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


#include "../inc/Msg_ValidSp.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    uint8_t mValid;
	
}Msg_ValidSp_t;


moduleMsg_t* Msg_ValidSpCreate(uint32_t destination, uint8_t msgNr
    , uint8_t valid)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_ValidSp_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned_e;
        msg->mMsgNr = msgNr;
        msg->type = Msg_ValidSp_e;
        msg->mAllocatedSize = size;

        Msg_ValidSp_t* internal_data = (Msg_ValidSp_t*)(msg + 1);
        
        internal_data->mValid = valid;

    }
    return msg;
}

uint8_t Msg_ValidSpGetValid(moduleMsg_t* msg)
{
    uint8_t value = {0};
    if(msg && (msg->type == Msg_ValidSp_e))
    {
        Msg_ValidSp_t* internal_data = (Msg_ValidSp_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mValid;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_ValidSpSetValid(moduleMsg_t* msg, uint8_t valid)
{
    if(msg && (msg->type == Msg_ValidSp_e))
    {
        Msg_ValidSp_t* internal_data = (Msg_ValidSp_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mValid  = valid;
        }
    }
    else
    {
       configASSERT(0);
    }
}

uint8_t* Msg_ValidSpSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_ValidSp_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_ValidSp_t* data = (Msg_ValidSp_t*)(msg + 1);
        if(data)
        {

        }
    }
    return buffer;
}

moduleMsg_t* Msg_ValidSpDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_ValidSp_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);
    if(msg)
    {
        msg->mAllocatedSize = size;
        uint8_t* bufferOrig = buffer;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        buffer_size -= buffer - bufferOrig;
        Msg_ValidSp_t* data = (Msg_ValidSp_t*)(msg + 1);
        if(data)
        {

        }
    }
    return msg;
}
