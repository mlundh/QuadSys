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
 * Handler function for subscription request event. This is a default handler.
 * @param obj         Current event handler object.
 * @param taskParam   Optional parameters from the task.
 * @param data        Event data.
 * @return            1 if ok, 0 otherwise.
 */
uint8_t subscriptionReq(eventHandler_t* obj, void* taskParam, eventData_t* data);

/**
 * Handler function for subscription request event. This is a default handler.
 * @param obj         Current event handler object.
 * @param taskParam   Optional parameters from the task.
 * @param data        Event data.
 * @return            1 if ok, 0 otherwise.
 */
uint8_t regHandlerReq(eventHandler_t* obj, void* taskParam, eventData_t* data);

/**
 * Handler function for subscription request event. This is a default handler.
 * @param obj         Current event handler object.
 * @param taskParam   Optional parameters from the task.
 * @param data        Event data.
 * @return            1 if ok, 0 otherwise.
 */
uint8_t meshComplete(eventHandler_t* obj, void* taskParam, eventData_t* data);

/**
 * Function to send an event to all registered handlers.
 * @param obj         Current event handler object.
 * @param event       The event to send.
 * @return            1 if it was possible to send to everyone.
 */
uint8_t sendEventHandlerBC(eventHandler_t* obj, event_t event);

eventHandler_t* createEventHandler(QueueHandle_t* masterQueue, uint8_t nr_handlers)
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
  obj->registeredHandlers = 0;
  obj->meshComplete = 0;
  obj->eventFcns[eRegisterHandler] = regHandlerReq;
  obj->eventFcns[eSubscribe] = subscriptionReq;
  obj->eventFcns[eUnsubscribe] = subscriptionReq;
  obj->eventFcns[eMeshComplete] = meshComplete;
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

uint8_t initEventHandler(eventHandler_t* obj)
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
      uint8_t result = receiveEvent(obj, NULL, portMAX_DELAY);
      if(!result)
      {
        return 0;
      }
    }
    // Master is responsible for the correct table in the init phase,
    // and has to add itself to the table to start with.
    obj->handlerSubscriptions[0] = obj->subscriptions;
    // Then send all the data to all other handlers.
    if(!sendEventHandlerBC(obj, eRegisterHandler))
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
    xSemaphoreTake(obj->semaphore, portMAX_DELAY);
    // Handle the response from the master.
    if(!receiveEvent(obj, NULL, portMAX_DELAY))
    {
      return 0;
    }

    // Handle the meshComplete event.
    if(!receiveEvent(obj, NULL, portMAX_DELAY))
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

uint8_t subscribeEvents(eventHandler_t* obj, uint32_t events)
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
  return sendEventHandlerBC(obj, eSubscribe);
}


uint8_t unsubscribeEvents(eventHandler_t* obj, uint32_t events)
{
  if(!obj)
  {
    return 0;
  }
  // Clear bits in subscription.
  uint32_t eventMask = ~events;
  obj->subscriptions &= eventMask;

  // subscribe is a broadcast message.
  return sendEventHandlerBC(obj, eUnsubscribe);
}

uint8_t sendEventHandlerBC(eventHandler_t* obj, event_t event)
{
  for(int i = 0; i < obj->nrHandlers; i++)
  {
    // Send gathered information to everyone except self.
    if(i != obj->handlerId)
    {
      eventData_t eventData = {0};
      eventData.eventNumber = event;
      eventData.data = obj;

      if(xQueueSend(obj->handlers[i], &eventData, EVENT_BLOCK_TIME)!= pdTRUE)
      {
        return 0;
      }
      // We are not allowed to use obj until the semaphore is given,
      // so wait until it is processed.
      xSemaphoreTake(obj->semaphore, portMAX_DELAY);
    }
  }
  return 1;
}


uint8_t sendEvent(eventHandler_t* obj, eventData_t event)
{
  // Use subscribeEvent to subscribe to an event, and
  // unsubscribeEvent to unsubscribe.
  if(!obj || (event.eventNumber == eSubscribe)
          || (event.eventNumber == eUnsubscribe)
          || (event.eventNumber == eRegisterHandler))
  {
    return 0;
  }
  // Send to everyone except self.
  for(int i = 0; i < obj->nrHandlers; i++)
  {
    if(obj->handlers[i] != obj->eventQueue)
    {
      uint32_t eventMask = (1 << event.eventNumber);
      uint8_t subscribed = obj->handlerSubscriptions[i] & eventMask;
      if(subscribed)
      {
        if(!xQueueSend(obj->handlers[i], &event, EVENT_BLOCK_TIME))
        {
          return 0;
        }
      }
    }
  }
  return 1;
}


uint8_t receiveEvent(eventHandler_t* obj, void* taskParam, uint32_t blockTime)
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
    if(obj->eventFcns[event_data.eventNumber])
    {
      obj->eventFcns[event_data.eventNumber](obj, taskParam, &event_data);
    }
    else
    {
      //Received unhandled event.
    }
  }
  else
  {
    return 0;
  }
  return 1;
}

uint8_t registerEventCallback(eventHandler_t* obj, event_t eventNumber, eventHandlerFcn fcn)
{
  if(eventNumber >= eNrEvents)
  {
    return 0;
  }
  obj->eventFcns[eventNumber] = fcn;
  return 1;
}

uint8_t subscriptionReq(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  if(!data)
  {
    return 0;
  }
  eventHandler_t* subsData = (eventHandler_t*)data->data;

  obj->handlerSubscriptions[subsData->handlerId] = subsData->subscriptions;
  xSemaphoreGive(subsData->semaphore);
  return 1;
}

uint8_t meshComplete(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  obj->meshComplete = 1;
  return 1;
}

uint8_t regHandlerReq(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  if(!data)
  {
    return 0;
  }
  eventHandler_t* subsData = (eventHandler_t*)data->data;

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
      xSemaphoreGive(subsData->semaphore);
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
