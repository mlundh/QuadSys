/*
 * event_handler.c
 *
 * Copyright (C) 2016 Martin Lundh
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


#include "Modules/EventHandler/inc/event_handler.h"
#include "Modules/Messages/inc/Msg_Subscriptions.h"
#define NO_ID (255)


/**
 * Internal function to distribute the new subscription to all nodes in the system.
 * @param obj           Current event handler object.
 * @param subscription  Full list of subscriptions of the current object.
 * @return
 */
uint8_t Event_SendSubscription(eventHandler_t* obj, uint32_t subscription);


/**
 * Internal function. Will fire the event in event_data if it matches event, or if bufferEvents is set to 0. If
 * the event is not fired then it will be written to the back of the circular buffer.
 * @param obj           Current event handler object.
 * @param event         Event to compare with.
 * @param found         Number of events found, external counter passed by pointer.
 * @param event_data    The event data to process.
 * @param bufferEvents  1 if events should be buffered, 0 otherwise.
 * @return              1 if ok, 0 otherwise.
 */
uint8_t Event_FireOrBufferEvent(eventHandler_t* obj, messageTypes_t event, uint8_t* found, moduleMsg_t* event_data, uint8_t bufferEvents);

/**
 * Send the specified event to all handlers, and wait for the same event from all other handlers.
 * Observe that ALL handlers need to subscribe to the event for this function to ever return.
 * @param obj         Current handler
 * @param event       The event to send and wait for. Do NOT use a complex message type with payload.
 * @return
 */
uint8_t Event_SendAndWaitForAll(eventHandler_t* obj, messageTypes_t event);

/**
 * Fire the callback registered to the event in event_data. The event_data will be
 * deleted after the callback has been fired. This means that all data that needs
 * to be used outside of the callback has to be copied.
 * @param obj         Current event handler object.
 * @param event_data  event number and data.
 * @return            1 of ok, 0 of no handler was registered.
 */

uint8_t Event_FireCB(eventHandler_t* obj, moduleMsg_t* event_data);

/**
 * Handler function for subscription request event. This is a default handler.
 * @param obj         Current event handler object.
 * @param data        Optional data to be used by the handler when firing the event.
 * @param eData        Event data.
 * @return            1 if ok, 0 otherwise.
 */
uint8_t Event_SubscriptionReq(eventHandler_t* obj, void* data, moduleMsg_t* eData);

/**
 * Default implementation of initialize that does nothing.
 * @param obj         Current event handler object.
 * @param data        Optional data to be used by the handler when firing the event.
 * @param eData        Event data.
 * @return            1.
 */
uint8_t Event_Initialize(eventHandler_t* obj, void* data, moduleMsg_t* eData);

/**
 * Default implementation of initialize done cb that does nothing.
 * @param obj         Current event handler object.
 * @param data        Optional data to be used by the handler when firing the event.
 * @param eData        Event data.
 * @return            1.
 */
uint8_t Event_InitializeDone(eventHandler_t* obj, void* data, moduleMsg_t* eData);


eventHandler_t* Event_CreateHandler(msgAddr_t id, uint8_t master)
{
    eventHandler_t* obj = pvPortMalloc(sizeof(eventHandler_t));
    if(!obj)
    {
        return NULL;
    }
    eventHandler_t tmpHandler = {0};
    *obj = tmpHandler;
    obj->eventQueue = xQueueCreate( EVENT_QUEUE_LENGTH,
            EVENT_QUEUE_ITEM_SIZE );
    obj->cBuffer = Event_CreateCBuff();
    obj->registeredHandlers = 0;
    for(int i = 0; i < Msg_LastType_e; i++)
    {
        obj->eventDataBinding[i] = NULL;
    }

    Event_RegisterCallback(obj, Msg_Subscriptions_e, Event_SubscriptionReq, NULL);
    Event_RegisterCallback(obj, e_Initialize_e, Event_Initialize, NULL);
    Event_RegisterCallback(obj, e_InitializeDone_e, Event_InitializeDone, NULL);

    obj->handlerId = id;

    if(master) // register self if we are master.
    {
        obj->master = 1;
        obj->handlers[obj->registeredHandlers] = obj;
        obj->handlerIds[obj->registeredHandlers] = obj->handlerId;
        obj->handlerSubscriptions[obj->registeredHandlers] = obj->subscriptions;
        obj->registeredHandlers++;
    }
    return obj;
}

void Event_DeleteHandler(eventHandler_t* obj)
{
    vQueueDelete(obj->eventQueue);
    Event_DeleteCBuff(obj->cBuffer);
    vPortFree(obj);
}

void Event_StartInitialize(eventHandler_t* obj)
{
    if(!Event_SendAndWaitForAll(obj, e_Initialize_e))
    {
        for(;;)
        {
            // ERROR!
        }
    }
}

void Event_EndInitialize(eventHandler_t* obj)
{
    if(!Event_SendAndWaitForAll(obj, e_InitializeDone_e))
    {
        for(;;)
        {
        }
    }
}


uint8_t Event_InitHandler(eventHandler_t* master, eventHandler_t* obj)
{
    if(!obj || !master || obj->master || !master->master)
    {
        return 0;
    }
    // add self to the list.

    // Check if this message handler has been registered before.
    for(int i = 0; i < NR_QUEUES_MAX; i++)
    {
        if((master->handlers[i] == obj) || (master->handlerIds[i] == obj->handlerId))
        {
            return 0;
        }
    }

    master->handlerSubscriptions[0] = master->subscriptions; // master is always in place 0.

    // We have not seen this handler before, increase the
    // number of registered handlers.
    // And register the handler.
    master->handlerSubscriptions[master->registeredHandlers] = obj->subscriptions;
    master->handlers[master->registeredHandlers] = obj;
    master->handlerIds[master->registeredHandlers] = obj->handlerId;
    master->registeredHandlers++;

    // Each time we register a new handler, we will spread the knowledge of that handler to all other handlers.
    for(int i = 0; i < master->registeredHandlers; i++)
    {
        for(int j = 0; j < NR_QUEUES_MAX; j++)
        {
            master->handlers[i]->handlers[j] = master->handlers[j];
            master->handlers[i]->handlerIds[j] = master->handlerIds[j];
            master->handlers[i]->handlerSubscriptions[j] = master->handlerSubscriptions[j];
        }
        master->handlers[i]->registeredHandlers = master->registeredHandlers;
    }
    return 1;
}

msgAddr_t Event_GetId(eventHandler_t* obj)
{
    if(!obj)
    {
        return Last_Address;
    }
    return obj->handlerId;
}

uint8_t Event_Subscribe(eventHandler_t* obj, messageTypes_t event)
{
    if(!obj)
    {
        return 0;
    }

    // sanity check. You have to have registered an event handler
    // function before you can subscribe to an event.

    if(!obj->eventFcns[event])
    {
        return 0;
    }
    obj->subscriptions &= (1 << event);
    obj->handlerSubscriptions[obj->handlerId] = obj->subscriptions;

    // subscribe is a broadcast message.
    return Event_SendSubscription(obj, obj->subscriptions);
}


uint8_t Event_Unsubscribe(eventHandler_t* obj, messageTypes_t event)
{
    if(!obj)
    {
        return 0;
    }

    // sanity check. You have to have registered an event handler
    // function before you unsubscribe for an event.

    if(!obj->eventFcns[event])
    {
        return 0;
    }
    // Clear bits in subscription.
    uint32_t eventMask = ~ (1 << event);
    obj->subscriptions &= eventMask;
    obj->handlerSubscriptions[obj->handlerId] = obj->subscriptions;

    // subscribe is a broadcast message.
    return Event_SendSubscription(obj, obj->subscriptions);
}

uint8_t Event_Send(eventHandler_t* obj, moduleMsg_t* msg)
{
    // Use subscribeEvent to subscribe to an event, and
    // unsubscribeEvent to unsubscribe.
    if(!obj || (msg->type == Msg_Subscriptions_e)
            || (msg->type == Msg_NoType_e)
            || (msg->type == Msg_LastType_e))
    {
        return 0;
    }
    uint8_t result = 1;
    Msg_SetSource(msg, obj->handlerId);
    if(msg->mDestination == Broadcast) // Broadcast, send to all that are subscribed.
    {
        for(int i = 0; i < obj->registeredHandlers; i++)
        {
            if(obj->handlerIds[i] != obj->handlerId) // do not send to self.
            {
                uint32_t eventMask = (1 << msg->type);
                uint32_t subscribed = obj->handlerSubscriptions[i] & eventMask;
                if(subscribed) // only send if the receiver has subscribed to this message.
                {
                    // clone the message so that all subscribers gets a unique message.
                    // TODO save number of subscribers so we can send the initial message.
                    moduleMsg_t* clone = Msg_Clone(msg);
                    if(!xQueueSendToBack(obj->handlers[i]->eventQueue, &clone, EVENT_BLOCK_TIME))
                    {
                        result = 0;
                        Msg_Delete(msg);
                        Msg_Delete(clone);
                    }
                }
            }
        }
        Msg_Delete(msg); // original message is deleted.
    }
    else // send only to the destination.
    {
        for(int i = 0; i < obj->registeredHandlers; i++)
        {
            if(obj->handlerIds[i] == msg->mDestination)
            {
                uint32_t eventMask = (1 << msg->type);
                uint32_t subscribed = obj->handlerSubscriptions[i] & eventMask;
                if(subscribed)
                {
                    if(!xQueueSendToBack(obj->handlers[i]->eventQueue, &msg, EVENT_BLOCK_TIME))
                    {
                        result = 0;
                        Msg_Delete(msg);
                    }
                }
                break;
            }
        }
    }

    return result;
}

uint8_t Event_SendSubscription(eventHandler_t* obj, uint32_t subscription)
{
    uint8_t result = 1;

    moduleMsg_t* msg = Msg_SubscriptionsCreate(Broadcast, 0, subscription);
    Msg_SetSource(msg, obj->handlerId);
    // Send to everyone except self.
    for(int i = 0; i < obj->registeredHandlers; i++)
    {
        if(obj->handlers[i] != obj)
        {
            uint32_t eventMask = (1 << Msg_Subscriptions_e);
            uint32_t subscribed = obj->handlerSubscriptions[i] & eventMask;
            if(subscribed)
            {
                if(!xQueueSendToBack(obj->handlers[i]->eventQueue, &msg, EVENT_BLOCK_TIME))
                {
                    result = 0;
                    Msg_Delete(msg);
                }
            }
        }
    }
    return result;
}

uint8_t Event_Receive(eventHandler_t* obj, uint32_t blockTime)
{
    if(!obj)
    {
        return 0;
    }
    // Handle all incoming events.
    moduleMsg_t* event_data = NULL;
    if(xQueueReceive(obj->eventQueue, &event_data, blockTime) == pdTRUE)
    {
        //If there is a handler for an event, then handle the event.
        if(!Event_FireCB(obj, event_data))
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

uint8_t Event_WaitForEvent(eventHandler_t* obj, messageTypes_t event, uint8_t waitForNr, uint8_t bufferEvents)
{
    if(!obj)
    {
        return 0;
    }
    uint8_t nr_found = 0;
    moduleMsg_t* event_data;
    uint8_t nr_buffered_events = Event_CBuffNrElemets(obj->cBuffer);
    // First handle all buffered events. Go through the whole buffer, and
    // if bufferEvents is true, then write the events that does not match the
    // event we are waiting for back to the buffer as they should be handled
    // later.
    for(uint8_t i = 0; i <  nr_buffered_events; i++)
    {
        if(Event_CBuffPopFront(obj->cBuffer, &event_data))
        {
            if(!Event_FireOrBufferEvent(obj, event, &nr_found, event_data, bufferEvents))
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }

    // Handle all new incoming events. Do this until we get the event we are waiting for.
    while(nr_found < waitForNr)
    {
        if(xQueueReceive(obj->eventQueue, &event_data, portMAX_DELAY) == pdTRUE)
        {
            if(!Event_FireOrBufferEvent(obj, event, &nr_found, event_data, bufferEvents))
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    // All events found.
    return 1;
}

uint8_t Event_FireOrBufferEvent(eventHandler_t* obj, messageTypes_t event, uint8_t* nr_found, moduleMsg_t* event_data, uint8_t bufferEvents)
{
    // handle the expected event.
    if(event_data->type == event)
    {
        (*nr_found)++;
        if(!Event_FireCB(obj, event_data))
        {
            return 0;
        }
    }
    // handle all other events. Buffer events if requested, handle immediately otherwise.
    else if(!bufferEvents)
    {
        //If there is a handler for an event, then handle the event.
        if(!Event_FireCB(obj, event_data))
        {
            return 0;
        }
    }
    else // buffer the event.
    {
        if(!Event_CBuffPushBack(obj->cBuffer, event_data))
        {
            return 0;
        }
    }
    return 1;
}


uint8_t Event_SendAndWaitForAll(eventHandler_t* obj, messageTypes_t event)
{
    moduleMsg_t* ev = Msg_Create(event, 0, 0, 0);
    // Note that Event_Send might get a context switch between the transmissions to all
    // handlers. This means that only sync up until this point is valid, and it is not
    // guaranteed that all handlers receive events in the same order.
    //TODO broadcast...

    if(!Event_Send(obj, ev))
    {
        return 0;
    }
    if(!Event_WaitForEvent(obj, event, obj->registeredHandlers - 1, 1))
    {
        return 0;
    }
    return 1;
}


uint8_t Event_HandleBufferedEvents(eventHandler_t* obj)
{
    moduleMsg_t* event_data;
    if(!Event_CBuffPopFront(obj->cBuffer, &event_data))
    {
        return 0;
    }
    if(!Event_FireCB(obj, event_data))
    {
        return 0;
    }
    return 1;
}

uint8_t Event_FireCB(eventHandler_t* obj, moduleMsg_t* event_data)
{
    if(obj->eventFcns[event_data->type])
    {
        obj->eventFcns[event_data->type](obj, obj->eventDataBinding[event_data->type], event_data);
    }
    else
    {
        //Received unhandled event.
        return 0;
    }
    Msg_Delete(event_data);
    return 1;
}


uint8_t Event_RegisterCallback(eventHandler_t* obj, messageTypes_t type, eventHandlerFcn fcn, void* data)
{
    if(!obj || type >= Msg_LastType_e)
    {
        return 0;
    }
    obj->eventDataBinding[type] = data;
    obj->eventFcns[type] = fcn;
    obj->subscriptions |= (1 << type); // Subscriptions are default on.

    return 1;
}

uint8_t Event_SubscriptionReq(eventHandler_t* obj, void* data, moduleMsg_t* eData)
{
    if(!eData)
    {
        return 0;
    }

    obj->handlerSubscriptions[Msg_GetSource(eData)] = Msg_SubscriptionsGetSubscriptions(eData);
    return 1;
}

uint8_t Event_Initialize(eventHandler_t* obj, void* data, moduleMsg_t* eData)
{
    return 1;
}

uint8_t Event_InitializeDone(eventHandler_t* obj, void* data, moduleMsg_t* eData)
{
    return 1;
}
