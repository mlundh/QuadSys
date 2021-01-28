/*
 * Msg_Subscriptions.h
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


#include "../inc/Msg_Subscriptions.h"
#include "FreeRTOS.h"
#include "task.h"

 typedef struct
{
    uint8_t* mSubscriptions;
	uint32_t mSubscriptionslength;
	uint32_t mSubscriptionsbufferlength;
	
}Msg_Subscriptions_t;


moduleMsg_t* Msg_SubscriptionsCreate(uint32_t destination, uint8_t msgNr
    , uint32_t Subscriptionsbufferlength)
{
    size_t mallocSize = sizeof(moduleMsg_t) + sizeof(Msg_Subscriptions_t)  + (Subscriptionsbufferlength);
    moduleMsg_t* msg = pvPortMalloc(mallocSize);

    if(msg)
    {
        msg->mDestination = destination;
        msg->mSource = Unassigned_e;
        msg->mMsgNr = msgNr;
        msg->type = Msg_Subscriptions_e;
        msg->mAllocatedSize = mallocSize;

        Msg_Subscriptions_t* internal_data = (Msg_Subscriptions_t*)(msg + 1);
        
        internal_data->mSubscriptions = (uint8_t*)(internal_data+1);
        internal_data->mSubscriptionslength = 0;
        internal_data->mSubscriptionsbufferlength = Subscriptionsbufferlength;

    }
    return msg;
}

uint8_t* Msg_SubscriptionsGetSubscriptions(moduleMsg_t* msg)
{
    uint8_t* value = {0};
    if(msg && (msg->type == Msg_Subscriptions_e))
    {
        Msg_Subscriptions_t* internal_data = (Msg_Subscriptions_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mSubscriptions;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_SubscriptionsSetSubscriptions(moduleMsg_t* msg, uint8_t* subscriptions)
{
    if(msg && (msg->type == Msg_Subscriptions_e))
    {
        Msg_Subscriptions_t* internal_data = (Msg_Subscriptions_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mSubscriptions  = subscriptions;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint32_t Msg_SubscriptionsGetSubscriptionslength(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_Subscriptions_e))
    {
        Msg_Subscriptions_t* internal_data = (Msg_Subscriptions_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mSubscriptionslength;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_SubscriptionsSetSubscriptionslength(moduleMsg_t* msg, uint32_t Subscriptionslength)
{
    if(msg && (msg->type == Msg_Subscriptions_e))
    {
        Msg_Subscriptions_t* internal_data = (Msg_Subscriptions_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mSubscriptionslength  = Subscriptionslength;
        }
    }
    else
    {
       configASSERT(0);
    }
}
uint32_t Msg_SubscriptionsGetSubscriptionsbufferlength(moduleMsg_t* msg)
{
    uint32_t value = {0};
    if(msg && (msg->type == Msg_Subscriptions_e))
    {
        Msg_Subscriptions_t* internal_data = (Msg_Subscriptions_t*)(msg + 1);
        if(internal_data)
        {
            value = internal_data->mSubscriptionsbufferlength;
        }
    }
    else
    {
       configASSERT(0);
    }
    return value;
}

void Msg_SubscriptionsSetSubscriptionsbufferlength(moduleMsg_t* msg, uint32_t Subscriptionsbufferlength)
{
    if(msg && (msg->type == Msg_Subscriptions_e))
    {
        Msg_Subscriptions_t* internal_data = (Msg_Subscriptions_t*)(msg + 1);
        if(internal_data)
        {
            internal_data->mSubscriptionsbufferlength  = Subscriptionsbufferlength;
        }
    }
    else
    {
       configASSERT(0);
    }
}

uint8_t* Msg_SubscriptionsSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size)
{
    if(msg && (msg->type == Msg_Subscriptions_e))
    {
        buffer = Msg_Serialize(msg, buffer, buffer_size);
        Msg_Subscriptions_t* data = (Msg_Subscriptions_t*)(msg + 1);
        if(data)
        {

        }
    }
    return buffer;
}

moduleMsg_t* Msg_SubscriptionsDeserialize(uint8_t* buffer, uint32_t buffer_size)
{
    size_t size = sizeof(moduleMsg_t) + sizeof(Msg_Subscriptions_t)  + (buffer_size);
    moduleMsg_t* msg = pvPortMalloc(size);
    if(msg)
    {
        msg->mAllocatedSize = size;
        uint8_t* bufferOrig = buffer;
        buffer = Msg_DeSerialize(msg, buffer, buffer_size);
        buffer_size -= buffer - bufferOrig;
        Msg_Subscriptions_t* data = (Msg_Subscriptions_t*)(msg + 1);
        if(data)
        {

        }
    }
    return msg;
}

