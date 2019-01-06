/*
 * state_handler.c
 *
 * Copyright (C) 2015 Martin Lundh
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
#include "FlightModeHandler/inc/flight_mode_handler.h"

#include "EventHandler/inc/event_handler.h"

struct FlightModeHandler
{
  SemaphoreHandle_t xMutex;
  QueueHandle_t current_mode_queue;
};

/**
 * Queue holding current state. Implemented
 * as a queue to ensure thread safety.
 */
#define STATE_CURRENT_QUEUE_LENGTH      (1)
#define STATE_CURRENT_QUEUE_ITEM_SIZE        (sizeof(mode_t))


/**
 * State lock queue. Use this to prevent state changes.
 * Implemented as a queue to ensure thread safety.
 */
#define STATE_LOCK_QUEUE_LENGTH      (1)
#define STATE_LOCK_QUEUE_ITEM_SIZE        (sizeof(uint8_t))

static BaseType_t FMode_ChangeAllowed(FlightModeHandler_t* obj, FMode_t state_req);
void FMode_SendEvent(FlightModeHandler_t*  obj, eventHandler_t* evHandler);

FlightModeHandler_t* FMode_CreateStateHandler()
{
  FlightModeHandler_t* obj = pvPortMalloc(sizeof(FlightModeHandler_t));

  obj->current_mode_queue = xQueueCreate( STATE_CURRENT_QUEUE_LENGTH,
      STATE_CURRENT_QUEUE_ITEM_SIZE );
  obj->xMutex = xSemaphoreCreateMutex();


  if( !obj->current_mode_queue
      || !obj->xMutex)
  {
    return NULL;
  }
  return obj;
}


/*Initialize queues and mutex used internally
 * by the state handler*/
void FMode_InitFModeHandler(FlightModeHandler_t*  obj, eventHandler_t* evHandler)
{
  FMode_t state = fmode_init;
  if(xQueueSendToBack(obj->current_mode_queue, &state, 2) != pdPASS)
  {
    //TODO send error!
  }
  FMode_SendEvent(obj, evHandler);
}

/**
 * Get the current state.
 */
FMode_t FMode_GetCurrent(FlightModeHandler_t*  obj)
{
  FMode_t state = fmode_not_available;
  if( !xQueuePeek(obj->current_mode_queue, &state, 0) )
  {
    //no state, error!
  }
  return state;
}

// TODO add fault text readable from remote.
BaseType_t FMode_Fault(FlightModeHandler_t*  obj, eventHandler_t* evHandler)
{
  FMode_t state_req = fmode_fault;
  //State_Lock();
  xQueueOverwrite(obj->current_mode_queue, &state_req);
  FMode_SendEvent(obj, evHandler);
  return pdTRUE;

}

BaseType_t FMode_Change(FlightModeHandler_t*  obj, eventHandler_t* evHandler, FMode_t state_req)
{
  BaseType_t result = pdFALSE;
  if( xSemaphoreTake( obj->xMutex, ( TickType_t )(2UL / portTICK_PERIOD_MS) ) == pdTRUE )
  {
    if(FMode_ChangeAllowed(obj, state_req) == pdTRUE)
    {
      result = xQueueOverwrite(obj->current_mode_queue, &state_req);
      FMode_SendEvent(obj, evHandler);
    }
    xSemaphoreGive(obj->xMutex);
  }
  /*Could not obtain mutex or change not allowed.*/
  return result;
}


static BaseType_t FMode_ChangeAllowed(FlightModeHandler_t* obj, FMode_t state_req)
{
  switch ( FMode_GetCurrent(obj) )
  {
  case fmode_init:
    if(    (state_req == fmode_disarming) )
    {
      return pdTRUE;
    }
    break;
  case fmode_disarmed:
    if(    (state_req == fmode_config)
        || (state_req == fmode_init )
        || (state_req == fmode_armed )
        || (state_req == fmode_arming )
        || (state_req == fmode_fault ) )
    {
      return pdTRUE;
    }
    break;
  case fmode_config:
    if(    (state_req == fmode_disarming)
        || (state_req == fmode_fault ) )
    {
      return pdTRUE;
    }
    break;
  case fmode_arming:
    if(    (state_req == fmode_armed)
        || (state_req == fmode_disarming )
        || (state_req == fmode_fault ) )
    {
      return pdTRUE;
    }
    break;
  case fmode_armed:
    if(    (state_req == fmode_disarming)
        || (state_req == fmode_fault ) )
    {
      return pdTRUE;
    }
    break;
  case   fmode_disarming:
    if(    (state_req == fmode_disarmed)
        || (state_req == fmode_fault ) )
    {
      return pdTRUE;
    }
    break;
  case fmode_fault:
    if(    (state_req == fmode_disarming) )
    {
      return pdTRUE;
    }
    break;
  default:
    return pdFALSE;
    break;
  }

  return pdFALSE;
}
void FMode_SendEvent(FlightModeHandler_t*  obj, eventHandler_t* evHandler)
{
  if(evHandler && obj)
  {
    eventData_t event;
    event.eventNumber = eNewFlightMode;
    event.data = obj;
    Event_Send(evHandler, event);
  }
}

FMode_t FMode_GetEventData(eventData_t* data)
{
  if(data && data->data)
  {
    FlightModeHandler_t* flightModeHanler = (FlightModeHandler_t*)data->data;
    FMode_t flightMode = FMode_GetCurrent(flightModeHanler);
    return flightMode;
  }
  return fmode_not_available;
}
