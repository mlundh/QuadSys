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
#define NO_ID (255)

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
uint8_t Event_FireOrBufferEvent(eventHandler_t* obj, event_t event, uint8_t* found, eventData_t event_data, uint8_t bufferEvents);


/**
 * Fire the callback registered to the event in event_data.
 * @param obj         Current event handler object.
 * @param event_data  event number and data.
 * @return            1 of ok, 0 of no handler was registered.
 */

uint8_t Event_FireCB(eventHandler_t* obj, eventData_t* event_data);

/**
 * Handler function for subscription request event. This is a default handler.
 * @param obj         Current event handler object.
 * @param data        Optional data to be used by the handler when firing the event.
 * @param eData        Event data.
 * @return            1 if ok, 0 otherwise.
 */
uint8_t Event_SubscriptionReq(eventHandler_t* obj, void* data, eventData_t* eData);

/**
 * Handler function for subscription request event. This is a default handler.
 * @param obj         Current event handler object.
 * @param data        Optional data to be used by the handler when firing the event.
 * @param eData        Event data.
 * @return            1 if ok, 0 otherwise.
 */
uint8_t Event_RegHandlerReq(eventHandler_t* obj, void* data, eventData_t* eData);

/**
 * Handler function for subscription request event. This is a default handler.
 * @param obj         Current event handler object.
 * @param data        Optional data to be used by the handler when firing the event.
 * @param eData        Event data.
 * @return            1 if ok, 0 otherwise.
 */
uint8_t Event_MeshComplete(eventHandler_t* obj, void* data, eventData_t* eData);

/**
 * Default implementation of initialize that does nothing.
 * @param obj         Current event handler object.
 * @param data        Optional data to be used by the handler when firing the event.
 * @param eData        Event data.
 * @return            1.
 */
uint8_t Event_Initialize(eventHandler_t* obj, void* data, eventData_t* eData);

/**
 * Default implementation of initialize done cb that does nothing.
 * @param obj         Current event handler object.
 * @param data        Optional data to be used by the handler when firing the event.
 * @param eData        Event data.
 * @return            1.
 */
uint8_t Event_InitializeDone(eventHandler_t* obj, void* data, eventData_t* eData);


/**
 * Function to send an event to all registered handlers.
 * @param obj         Current event handler object.
 * @param event       The event to send.
 * @return            1 if it was possible to send to everyone.
 */
uint8_t Event_SendHandlerBC(eventHandler_t* obj, event_t event);

eventHandler_t* Event_CreateHandler(QueueHandle_t masterQueue, uint8_t nr_handlers)
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
  obj->semaphore = xSemaphoreCreateBinary();
  obj->cBuffer = Event_CreateCBuff();
  obj->registeredHandlers = 0;
  obj->meshComplete = 0;
  for(int i = 0; i < eNrEvents; i++)
  {
    obj->eventDataBinding[i] = NULL;
  }
  obj->eventFcns[eRegisterHandler] = Event_RegHandlerReq;
  obj->eventFcns[eSubscribe] = Event_SubscriptionReq;
  obj->eventFcns[eUnsubscribe] = Event_SubscriptionReq;
  obj->eventFcns[eMeshComplete] = Event_MeshComplete;
  obj->eventFcns[eInitialize] = Event_Initialize;
  obj->eventFcns[eInitializeDone] = Event_InitializeDone;
  obj->subscriptions = (1 << eInitialize) | (1 << eInitializeDone);
  if(masterQueue) // We are not the master.
  {
    obj->handlers[0] = masterQueue;
    obj->handlerId = NO_ID;
    obj->nrHandlers = NO_ID;
  }
  else // We are the master
  {
    obj->handlers[0] = obj->eventQueue;
    obj->handlerId = 0;
    obj->nrHandlers = nr_handlers;
  }
  return obj;
}

uint8_t Event_InitHandler(eventHandler_t* obj)
{
  if(!obj)
  {
    return 0;
  }
  if(obj->handlerId == 0) // We are master
  {
    // Wait for all normal handlers to register. -1 since we are a handler ourselfs.
    while((obj->nrHandlers-1) != obj->registeredHandlers)
    {
      uint8_t result = Event_Receive(obj, portMAX_DELAY);
      if(!result)
      {
        return 0;
      }
    }
    // Master is responsible for the correct table in the init phase,
    // and has to add itself to the table to start with.
    obj->handlerSubscriptions[0] = obj->subscriptions;
    // Then send all the data to all other handlers.
    if(!Event_SendHandlerBC(obj, eRegisterHandler))
    {
      return 0;
    }

    // Send MeshComplete to everyone.
    eventData_t meshComplete = {0};
    meshComplete.eventNumber = eMeshComplete;
    for(int i = 0; i < obj->nrHandlers; i++)
    {
      if(obj->handlers[i] != obj->eventQueue)
      {
        if(!xQueueSend(obj->handlers[i], &meshComplete, EVENT_BLOCK_TIME))
        {
          //TODO error;
        }
      }
    }
    obj->meshComplete = 1;

  }
  else // we are not master.
  {
    eventData_t event = {0};
    event.eventNumber = eRegisterHandler;
    event.data = obj;
    // Send to master.
    if(xQueueSend(obj->handlers[0], &event, portMAX_DELAY) != pdTRUE)
    {
      return 0;
    }
    // We are not allowed to use obj until the semaphore is given,
    // so wait until it is processed.
    if(xSemaphoreTake(obj->semaphore, portMAX_DELAY) != pdTRUE)
    {
      return 0;
    }
    // Handle the response from the master.
    if(!Event_Receive(obj, portMAX_DELAY))
    {
      return 0;
    }

    // Handle the meshComplete event.
    if(!Event_Receive(obj, portMAX_DELAY))
    {
      return 0;
    }
    if(!obj->meshComplete)
    {
      return 0;
    }

  }
  return 1;
}

uint8_t Event_Subscribe(eventHandler_t* obj, uint32_t events)
{
  if(!obj)
  {
    return 0;
  }

  // sanity check. You have to have registered an event handler
  // function before you register for an event.
  for(uint32_t i = 0; i < eNrEvents; i++)
  {
    uint32_t mask = ~(1 << i);
    if(events & mask)
    {
      if(!obj->eventFcns[i])
      {
        return 0;
      }
    }
  }
  obj->subscriptions = events;
  obj->handlerSubscriptions[obj->handlerId] = events;

  // subscribe is a broadcast message.
  return Event_SendHandlerBC(obj, eSubscribe);
}


uint8_t Event_Unsubscribe(eventHandler_t* obj, uint32_t events)
{
  if(!obj)
  {
    return 0;
  }
  // Clear bits in subscription.
  uint32_t eventMask = ~events;
  obj->subscriptions &= eventMask;

  // subscribe is a broadcast message.
  return Event_SendHandlerBC(obj, eUnsubscribe);
}

uint8_t Event_SendHandlerBC(eventHandler_t* obj, event_t event)
{
  for(int i = 0; i < obj->nrHandlers; i++)
  {
    // Send gathered information to everyone except self.
    if(i != obj->handlerId)
    {
      eventData_t eventData = {0};
      eventData.eventNumber = event;
      eventData.data = obj;

      if(xQueueSendToBack(obj->handlers[i], &eventData, EVENT_BLOCK_TIME)!= pdTRUE)
      {
        return 0;
      }
      // We are not allowed to use obj until the semaphore is given,
      // so wait until it is processed.
      if(xSemaphoreTake(obj->semaphore, portMAX_DELAY) != pdTRUE)
      {
        return 0;
      }
    }
  }
  return 1;
}


uint8_t Event_Send(eventHandler_t* obj, eventData_t event)
{
  // Use subscribeEvent to subscribe to an event, and
  // unsubscribeEvent to unsubscribe.
  if(!obj || (event.eventNumber == eSubscribe)
      || (event.eventNumber == eUnsubscribe)
      || (event.eventNumber == eRegisterHandler))
  {
    return 0;
  }
  uint8_t result = 1;
  // Send to everyone except self.
  for(int i = 0; i < obj->nrHandlers; i++)
  {
    if(obj->handlers[i] != obj->eventQueue)
    {
      uint32_t eventMask = (1 << event.eventNumber);
      uint32_t subscribed = obj->handlerSubscriptions[i] & eventMask;
      if(subscribed)
      {
        if(!xQueueSendToBack(obj->handlers[i], &event, EVENT_BLOCK_TIME))
        {
          result = 0;
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
  eventData_t event_data = {0};
  if(xQueueReceive(obj->eventQueue, &event_data, blockTime) == pdTRUE)
  {
    //If there is a handler for an event, then handle the event.
    if(!Event_FireCB(obj, &event_data))
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

uint8_t Event_WaitForEvent(eventHandler_t* obj, event_t event, uint8_t waitForNr, uint8_t bufferEvents)
{
  if(!obj)
  {
    return 0;
  }
  uint8_t nr_found = 0;
  eventData_t event_data = {0};
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

uint8_t Event_FireOrBufferEvent(eventHandler_t* obj, event_t event, uint8_t* nr_found, eventData_t event_data, uint8_t bufferEvents)
{
  // handle the expected event.
  if(event_data.eventNumber == event)
  {
    (*nr_found)++;
    if(!Event_FireCB(obj, &event_data))
    {
      return 0;
    }
  }
  // handle all other events. Buffer events if requested, handle immediately otherwise.
  else if(!bufferEvents)
  {
    //If there is a handler for an event, then handle the event.
    if(!Event_FireCB(obj, &event_data))
    {
      return 0;
    }
  }
  else // buffer the event.
  {
    if(!Event_CBuffPushBack(obj->cBuffer, &event_data))
    {
      return 0;
    }
  }
  return 1;
}


uint8_t Event_SendAndWaitForAll(eventHandler_t* obj, event_t event)
{
  eventData_t ev = {0};
  ev.eventNumber = event;
  // Note that Event_Send might get a context switch between the transmissions to all
  // handlers. This means that only sync up until this point is valid, and it is not
  // guaranteed that all handlers receive events in the same order.
  if(!Event_Send(obj, ev))
  {
    return 0;
  }
  if(!Event_WaitForEvent(obj, event, obj->nrHandlers - 1, 1))
  {
    return 0;
  }
  return 1;
}


uint8_t Event_HandleBufferedEvents(eventHandler_t* obj)
{
  eventData_t event_data = {0};
  if(!Event_CBuffPopFront(obj->cBuffer, &event_data))
  {
    return 0;
  }
  if(!Event_FireCB(obj, &event_data))
  {
    return 0;
  }
  return 1;
}

uint8_t Event_FireCB(eventHandler_t* obj, eventData_t* event_data)
{
  if(obj->eventFcns[event_data->eventNumber])
  {
    obj->eventFcns[event_data->eventNumber](obj, obj->eventDataBinding[event_data->eventNumber], event_data);
  }
  else
  {
    //Received unhandled event.
    return 0;
  }
  return 1;
}


uint8_t Event_RegisterCallback(eventHandler_t* obj, event_t eventNumber, eventHandlerFcn fcn, void* data)
{
  if(eventNumber >= eNrEvents)
  {
    return 0;
  }
  obj->eventDataBinding[eventNumber] = data;
  obj->eventFcns[eventNumber] = fcn;
  return 1;
}

uint8_t Event_SubscriptionReq(eventHandler_t* obj, void* data, eventData_t* eData)
{
  if(!eData)
  {
    return 0;
  }
  eventHandler_t* subsData = (eventHandler_t*)eData->data;

  obj->handlerSubscriptions[subsData->handlerId] = subsData->subscriptions;
  xSemaphoreGive(subsData->semaphore);
  return 1;
}

uint8_t Event_MeshComplete(eventHandler_t* obj, void* data, eventData_t* eData)
{
  obj->meshComplete = 1;
  return 1;
}

uint8_t Event_RegHandlerReq(eventHandler_t* obj, void* data, eventData_t* eData)
{
  if(!eData)
  {
    return 0;
  }
  eventHandler_t* subsData = (eventHandler_t*)eData->data;

  if(obj->handlerId == 0) // we are master
  {
    // Master receives messages from all handlers first.
    // These messages are stored in the handler list.
    if(subsData->handlerId == 0)
    {
      return 0; // Only one master is allowed.
    }
    // Check if this message handler has been registered before.
    for(int i = 0; i < obj->nrHandlers; i++)
    {
      if(obj->handlers == subsData->eventQueue)
      {
        xSemaphoreGive(subsData->semaphore);
        return 0;// We are not allowed to register more than once.
      }
    }
    // We have not seen this handler before, increase the
    // number of registered handlers.
    obj->registeredHandlers++;
    // And register the handler.
    obj->handlerSubscriptions[obj->registeredHandlers] = subsData->subscriptions;
    obj->handlers[obj->registeredHandlers] = subsData->eventQueue;
    xSemaphoreGive(subsData->semaphore);
  }
  else // We are a normal handler.
  {
    // A normal handler will only receive the registerHandler request
    // once, and the data should contain all valid Handlers.

    // Only the master is allowed to send a RegisterHandlerRequest to a normal
    // handler.
    if(subsData->handlerId != 0)
    {
      //xSemaphoreGive(subsData->semaphore);
      return 0;
    }
    // Copy the mesh data from the incoming transaction to the current object.
    for(int i = 0; i < subsData->nrHandlers; i++)
    {
      obj->handlerSubscriptions[i] = subsData->handlerSubscriptions[i];
      obj->handlers[i] = subsData->handlers[i];
      // We get an id from the master.
      if(obj->eventQueue == obj->handlers[i])
      {
        obj->handlerId = i;
      }
    }
    obj->nrHandlers = subsData->nrHandlers;
    xSemaphoreGive(subsData->semaphore);
    if(obj->handlerId == NO_ID)
    {
      return 0;
    }
  }
  return 1;
}

uint8_t Event_Initialize(eventHandler_t* obj, void* data, eventData_t* eData)
{
  return 1;
}

uint8_t Event_InitializeDone(eventHandler_t* obj, void* data, eventData_t* eData)
{
  return 1;
}
