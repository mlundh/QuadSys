/*
 * LogStop.h
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


#include "../inc/LogStop.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    
}LogStop_t;


moduleMsg_t* LogStopCreate(uint32_t destination, uint8_t msgNr
    )
{
    size_t size = sizeof(moduleMsg_t) + sizeof(LogStop_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned_e;
        msg->mMsgNr = msgNr;
        msg->type = LogStop_e;
        msg->mAllocatedSize = size;

        LogStop_t* internal_data = (LogStop_t*)(msg + 1);
        

    }
    return msg;
}


uint8_t* LogStopSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == LogStop_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        LogStop_t* data = (LogStop_t*)(msg + 1);
        if(data)
        {

        }
    }
    return buffer;
}

moduleMsg_t* LogStopDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(LogStop_t) ;
    moduleMsg_t* msg = pvPortMalloc(size);
    if(msg)
    {
        msg->mAllocatedSize = buffer_size;
        uint8_t* bufferOrig = buffer;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        buffer_size -= buffer - bufferOrig;
        LogStop_t* data = (LogStop_t*)(msg + 1);
        if(data)
        {

        }
    }
    return msg;
}

