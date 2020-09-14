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


#include "EventHandler/inc/event_handler.h"
#include "Messages/inc/Msg_Subscriptions.h"
#include "AppLog/inc/AppLog.h"
#define NO_ID (255)

/**
 * Send to the current node. The address of the message should correspond to the current segment.
 * @param obj			Current event handler object.
 * @param msg			The message to be sent.
 * @return
 */
uint8_t Event_SendInternal(eventHandler_t* obj, moduleMsg_t* msg);


/**
 * Send a broadcast to the local segment.
 * @param obj		Current event handler object.
 * @param msg		The message to be broadcast.
 * @param msg       Delete the original message after send. Normally this should be done.
 * @return
 */
uint8_t Event_SendBCInternal(eventHandler_t* obj, moduleMsg_t* msg, uint8_t delete);

/**
 * Send the message to a different segment.
 * @param obj		Current event handler object.
 * @param msg		The message to be transfered to a different segment.
 * @return
 */
uint8_t Event_SendExternal(eventHandler_t* obj, moduleMsg_t* msg);

/**
 * Internal function to distribute the new subscription to all nodes in the system.
 * @param obj           Current event handler object.
 * @return
 */
uint8_t Event_SendSubscription(eventHandler_t* obj);

/**
 * @brief Internal function for copying the subscription from one handler to the other. 
 * 
 * This function will copy the length defined in the macro NR_SUBSCRIPTIONS_MAX.
 * 
 * @param copyTo    Copy to this subscription array. 
 * @param copyFrom  Copy subscriptions from this subscription array. 
 */
void Event_CopySubscription(uint32_t* copyTo, uint32_t* copyFrom);

/**
 * @brief Insert the subscription into the objects subscription array.
 * 
 * @param obj   Event handler to insert a subscription into.
 * @param type  The message type to subscribe to.
 * @return      1 if success, 0 if no slots are free for new subscriptions.
 */
uint8_t Event_InsertSubscription(eventHandler_t* obj, messageTypes_t type);

/**
 * @brief Remove the subscription specified.
 * 
 * @param obj   Event handler to remove subscription from.
 * @param type  The message type to stop subscription of.
 */
void Event_RemoveSubscription(eventHandler_t* obj, messageTypes_t type);

/**
 * @brief Function to determine if the current event handler is subscribed to the message type. 
 *      
 * @param obj   Current event handler object.  
 * @param type  Message type to check.
 * @return uint8_t  1 if subscribed, 0 if not subscribed. 
 */
uint8_t Event_IsSubscribed(uint32_t* subscriptions, messageTypes_t type);

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
    obj->initialized = 0;
    for(int i = 0; i < Msg_LastType_e; i++)
    {
        obj->eventDataBinding[i] = NULL;
    }
    obj->portFcn = NULL;
    obj->portQueue = NULL;

    Event_RegisterCallback(obj, Msg_Subscriptions_e, Event_SubscriptionReq, NULL);

    obj->handlerId = id;

    if(master) // register self if we are master.
    {
        obj->master = 1;
        obj->handlers[obj->registeredHandlers] = obj;
        obj->handlerIds[obj->registeredHandlers] = obj->handlerId;
        Event_CopySubscription(obj->handlerSubscriptions[obj->registeredHandlers], obj->subscriptions);
        obj->registeredHandlers++;
    }
    return obj;
}


void Event_RegisterPortFunction(eventHandler_t* obj, eventHandlerFcn fcn, void* data)
{
    if(!obj || !fcn || obj->portFcn || !data)
    {
        return;
    }
    obj->portQueue = xQueueCreate( EVENT_QUEUE_LENGTH,
            EVENT_QUEUE_ITEM_SIZE );
    obj->portFcn = fcn;
    obj->portDataBinding = data;
}

void Event_DeleteHandler(eventHandler_t* obj)
{
    vQueueDelete(obj->eventQueue);
    if(obj->portQueue)
    {  
        vQueueDelete(obj->portQueue);
    }
    Event_DeleteCBuff(obj->cBuffer);
    vPortFree(obj);
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
    Event_CopySubscription(master->handlerSubscriptions[0], master->subscriptions);// master is always in place 0.

    // We have not seen this handler before, increase the
    // number of registered handlers.
    // And register the handler.
    Event_CopySubscription(master->handlerSubscriptions[master->registeredHandlers], obj->subscriptions);
    master->handlers[master->registeredHandlers] = obj;
    master->handlerIds[master->registeredHandlers] = obj->handlerId;
    master->registeredHandlers++;

    // Spread the knowledge of the port. First to register wins.
    if(master->portQueue) // If the master knows of the port queue, write it!
    {
        obj->portQueue = master->portQueue;
    }
    else if(obj->portFcn) // Else if the new handler has a port function, then we copy the queue to all known handlers.
    {
        master->portQueue = obj->portQueue;
        for(int j = 0; j < master->registeredHandlers; j++)
        {
            master->handlers[j]->portQueue = obj->portQueue;
        }
    }

    // Each time we register a new handler, we will spread the knowledge of that handler to all other handlers.
    for(int i = 0; i < master->registeredHandlers; i++)
    {
        for(int j = 0; j < NR_QUEUES_MAX; j++)
        {
            master->handlers[i]->handlers[j] = master->handlers[j];
            master->handlers[i]->handlerIds[j] = master->handlerIds[j];
            Event_CopySubscription(master->handlers[i]->handlerSubscriptions[j], master->handlerSubscriptions[j]);
        }
        master->handlers[i]->registeredHandlers = master->registeredHandlers;
    }
    master->initialized = 1;
    obj->initialized = 1;
    return 1;
}

msgAddr_t Event_GetId(eventHandler_t* obj)
{
    if(!obj)
    {
        return Unassigned_e;
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
    Event_InsertSubscription(obj, event);

    // subscribe is a broadcast message.
    return Event_SendSubscription(obj);
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
    Event_RemoveSubscription(obj, event);
    // subscribe is a broadcast message.
    return Event_SendSubscription(obj);
}

uint8_t Event_Send(eventHandler_t* obj, moduleMsg_t* msg)
{
        Msg_SetSource(msg, obj->handlerId);
        uint8_t result = Event_SendGeneric(obj, msg);
        return result;
}

uint8_t Event_SendGeneric(eventHandler_t* obj, moduleMsg_t* msg)
{
    // Use subscribeEvent to subscribe to an event, and
    // unsubscribeEvent to unsubscribe.
    if(!obj || !msg 
            || (msg->type == Msg_Subscriptions_e)
            || (msg->type == Msg_NoType_e)
            || (msg->type == Msg_LastType_e))
    {
        Msg_Delete(&msg);
        return 0;
    }
    uint8_t result = 1;
    if((msg->mDestination & REGION_MASK) == BC_e) // Message is addressed to the global segment.
    {
        if(msg->mDestination == Broadcast_e) // Global broadcast
        {
            result = Event_SendBCInternal(obj, msg, 0);
            if(result)
            {
                result = Event_SendExternal(obj, msg);
            }
        }
        else
        {
            // We got a message without destination...
        }
    }
    else if((msg->mDestination & REGION_MASK) == (obj->handlerId & REGION_MASK)) // Message is addressed to this segment.
    {
        if(msg->mDestination == FC_Broadcast_e)// Local broadcast
        {
            result = Event_SendBCInternal(obj, msg, 1);
        }
        else
        {
            result = Event_SendInternal(obj, msg);
        }
    }
    else // Message not addressed to this segment, send it to other segment to be handled.
    {
        result = Event_SendExternal(obj, msg);
    }

    return result;
}
uint8_t Event_SendInternal(eventHandler_t* obj, moduleMsg_t* msg)
{
    if(!obj || !msg)
    {
        return 0;
    }
    uint8_t result = 0;
    for(int i = 0; i < obj->registeredHandlers; i++)
    {
        if(obj->handlerIds[i] == msg->mDestination)
        {
            uint8_t subscribed = Event_IsSubscribed(obj->handlerSubscriptions[i], msg->type);
            if(subscribed)
            {
                if(!xQueueSendToBack(obj->handlers[i]->eventQueue, &msg, EVENT_BLOCK_TIME))
                {
                    result = 0;
                    
                    if(msg->mDestination != FC_SerialIOtx_e) // We should not spam FC_SerialIOtx_e, if that is what failed.
                    {
                        LOG_ENTRY(FC_SerialIOtx_e, obj, "Event: Error. Failed sendToBack SendInternal.");
                    }
                    Msg_Delete(&msg);
                    
                }
                else
                {
                    result = 1;
                }
            }
            break;
        }
    }
    if(!result)
    {
        Msg_Delete(&msg);
    }
    return result;
}


uint8_t Event_SendBCInternal(eventHandler_t* obj, moduleMsg_t* msg, uint8_t delete)
{
    if(!obj || !msg)
    {
        return 0;
    }
    uint8_t result = 1;
    for(int i = 0; i < obj->registeredHandlers; i++)
    {
        //Send to all, including self.
        uint8_t subscribed = Event_IsSubscribed(obj->handlerSubscriptions[i], msg->type);
        if(subscribed) // only send if the receiver has subscribed to this message.
        {
            // clone the message so that all subscribers gets a unique message.
            // TODO save number of subscribers so we can send the initial message.
            moduleMsg_t* clone = Msg_Clone(msg);
            if(!xQueueSendToBack(obj->handlers[i]->eventQueue, &clone, EVENT_BLOCK_TIME))
            {
                LOG_ENTRY(FC_SerialIOtx_e, obj, "Event: Error. Failed sendToBack BC internal.");
                result = 0;
                Msg_Delete(&msg);
                Msg_Delete(&clone);
            }
        }
          }
    if(delete)
    {
        Msg_Delete(&msg); // original message is deleted.
    }
    return result;
}
uint8_t Event_SendExternal(eventHandler_t* obj, moduleMsg_t* msg)
{
    if(!obj || !msg || !obj->portQueue)
    {
        return 0;
    }
    uint8_t result = 1;
   
    if(!xQueueSendToBack(obj->portQueue, &msg, EVENT_BLOCK_TIME))
    {
        LOG_ENTRY(FC_SerialIOtx_e, obj, "Event: Error. Failed send to external queue.");
        result = 0;
        Msg_Delete(&msg);
    }
    return result;
}

uint8_t Event_SendSubscription(eventHandler_t* obj)
{
    uint8_t result = 0;
    moduleMsg_t* msg = Msg_SubscriptionsCreate(FC_Broadcast_e, 0, NR_SUBSCRIPTIONS_MAX*sizeof(uint32_t));

    uint8_t* payload = Msg_SubscriptionsGetSubscriptions(msg);
    Event_CopySubscription((uint32_t*)payload, obj->subscriptions);

    Msg_SetSource(msg, obj->handlerId);
    // Send to everyone except self.

    Event_SendBCInternal(obj, msg, 1);
    
    return result;
}

uint8_t Event_ReceiveExternal(eventHandler_t* obj, uint32_t blockTime)
{
    if(!obj || !obj->portFcn)
    {
        return 0;
    }
    // Handle all incoming events.
    moduleMsg_t* event_data = NULL;
    if(xQueueReceive(obj->portQueue, &event_data, blockTime) == pdTRUE)
    {
        obj->portFcn(obj, obj->portDataBinding, event_data);
        Msg_Delete(&event_data);
    }
    else
    {
        return 0;
    }
    return 1;
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

uint8_t Event_WaitForEvent(eventHandler_t* obj, messageTypes_t event, uint8_t waitForNr, uint8_t bufferEvents, uint32_t blockTimeMs)
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

    uint32_t blockTicks = blockTimeMs * portTICK_PERIOD_MS;
    uint32_t remainingWaitTime = blockTicks;

    uint32_t timeBeforeWait = xTaskGetTickCount();

    // Handle all new incoming events. Do this until we get the event we are waiting for.
    while((nr_found < waitForNr) && (remainingWaitTime != 0))
    {
        if(xQueueReceive(obj->eventQueue, &event_data, remainingWaitTime) == pdTRUE)
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
        uint32_t timeAfterWait = xTaskGetTickCount();
        uint32_t waitedTicks = timeAfterWait - timeBeforeWait;
        if(waitedTicks >= blockTicks)
        {
            remainingWaitTime = 0;
        }
        else
        {
            remainingWaitTime = blockTicks - waitedTicks;
        }
    }
    // All events found.
    return 1;
}

void Event_CopySubscription(uint32_t* copyTo, uint32_t* copyFrom)
{
    if(!copyTo || !copyFrom)
    {
        return;
    }
    memcpy(copyTo, copyFrom, NR_SUBSCRIPTIONS_MAX * sizeof(uint32_t));
}


uint8_t Event_InsertSubscription(eventHandler_t* obj, messageTypes_t type)
{
    if(!obj)
    {
        return 0;
    }
    if(Event_IsSubscribed(obj->subscriptions, type))
    {
        return 1;
    }

    uint8_t result = 0;
    for (size_t i = 0; i < NR_SUBSCRIPTIONS_MAX; i++)
    {
        if(obj->subscriptions[i] == Msg_NoType_e)
        {
            obj->subscriptions[i] = type;
            result = 1;
            break;
        }
    }
    if(!result)
    {
        if(obj->initialized)
        {
            LOG_ENTRY(FC_SerialIOtx_e, obj, "%d Failed to subscribe to message: %d", obj->handlerId, type);
        }
        else
        {
            // Failed to insert subscription, and the handlers have not yet been initialized, meaning we can not
            // expect logging to work properly.
            configASSERT(0); 
        }
        
        
    }
    return result;
}


void Event_RemoveSubscription(eventHandler_t* obj, messageTypes_t type)
{
    if(!obj)
    {
        return;
    }
    for (size_t i = 0; i < NR_SUBSCRIPTIONS_MAX; i++)
    {
        if(obj->subscriptions[i] == type)
        {
            obj->subscriptions[i] = Msg_NoType_e;
            break;
        }
    }
}


uint8_t Event_IsSubscribed(uint32_t* subscriptions, messageTypes_t type)
{
    if(!subscriptions)
    {
        return 0;
    }
    uint8_t result = 0;
    for (size_t i = 0; i < NR_SUBSCRIPTIONS_MAX; i++)
    {
        if(subscriptions[i] == type)
        {
            result = 1;
            break;
        }
    }
    return result;
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
    Msg_Delete(&event_data);
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
    Event_Subscribe(obj, type); // Subscriptions are default on.
    return 1;
}

uint8_t Event_SubscriptionReq(eventHandler_t* obj, void* data, moduleMsg_t* eData)
{
    if(!eData)
    {
        return 0;
    }
    Event_CopySubscription(&(obj->handlerSubscriptions[Msg_GetSource(eData)][0]), (uint32_t*)Msg_SubscriptionsGetSubscriptions(eData));
    return 1;
}
