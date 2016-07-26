/*
 * control_mode_handler.c
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
#include "../inc/control_mode_handler.h"


#include "EventHandler/inc/event_handler.h"



struct CtrlModeHandler
{
  SemaphoreHandle_t xMutex;
  QueueHandle_t Mode_current_queue;
};


/**
 * Queue holding current mode. Implemented
 * as a queue to ensure thread safety.
 */
#define MODE_CURRENT_QUEUE_LENGTH      (1)
#define MODE_CURRENT_QUEUE_ITEM_SIZE        (sizeof(CtrlMode_t))


static uint8_t Ctrl_ModeChangeAllowed(CtrlModeHandler_t* obj, CtrlMode_t mode_req);
void Ctrl_SendEvent(CtrlModeHandler_t*  obj, eventHandler_t* evHandler);

CtrlModeHandler_t* Ctrl_CreateModeHandler()
{
  CtrlModeHandler_t* obj = pvPortMalloc(sizeof(CtrlModeHandler_t));
  if(!obj)
  {
    return NULL;
  }

  obj->Mode_current_queue = xQueueCreate( MODE_CURRENT_QUEUE_LENGTH,
        MODE_CURRENT_QUEUE_ITEM_SIZE );
  obj->xMutex = xSemaphoreCreateMutex();


    if(!obj->Mode_current_queue || !obj->xMutex)
    {
      return NULL;
    }
    return obj;
}


void Ctrl_InitModeHandler(CtrlModeHandler_t* obj, eventHandler_t* evHandler)
{

  CtrlMode_t current_mode = Control_mode_rate;
  if(xQueueSendToBack(obj->Mode_current_queue, &current_mode, 2) != pdPASS)
  {
    //TODO send error!
  }
  Ctrl_SendEvent(obj, evHandler);

}

/**
 * Get the current mode.
 */
CtrlMode_t Ctrl_GetCurrentMode(CtrlModeHandler_t* obj)
{
  CtrlMode_t mode = Control_mode_not_available;
  if( !xQueuePeek(obj->Mode_current_queue, &mode, 0) )
  {
    //no mode, error!
  }
  return mode;
}

// TODO add fault text readable from remote.
uint8_t Ctrl_FaultMode(CtrlModeHandler_t* obj, eventHandler_t* evHandler)
{
  if( xSemaphoreTake( obj->xMutex, ( TickType_t )(10UL / portTICK_PERIOD_MS) ) == pdTRUE )
  {
    CtrlMode_t mode_req = Control_mode_not_available;
    xQueueOverwrite(obj->Mode_current_queue, &mode_req);
    Ctrl_SendEvent(obj, evHandler);
    xSemaphoreGive(obj->xMutex);
  }
  return 1;

}

uint8_t Ctrl_ChangeMode(CtrlModeHandler_t* obj, eventHandler_t* evHandler, CtrlMode_t mode_req)
{
  uint8_t result = 0;
  if( xSemaphoreTake( obj->xMutex, ( TickType_t )(2UL / portTICK_PERIOD_MS) ) == pdTRUE )
  {
    if(Ctrl_ModeChangeAllowed(obj, mode_req) == pdTRUE)
    {
      result = xQueueOverwrite(obj->Mode_current_queue, &mode_req);
      Ctrl_SendEvent(obj, evHandler);
    }
    xSemaphoreGive(obj->xMutex);
  }
  /*Could not obtain mutex or change not allowed.*/
  return result;
}

static uint8_t Ctrl_ModeChangeAllowed(CtrlModeHandler_t* obj, CtrlMode_t mode_req)
{
  switch ( Ctrl_GetCurrentMode(obj) )
  {
  case Control_mode_rate:
    if(    (mode_req == Control_mode_attitude) )
    {
      return 1;
    }
    break;
  case Control_mode_attitude:
    if(    (mode_req == Control_mode_rate))
    {
      return 1;
    }
    break;
  default:
    return 1;
    break;
  }

  return 0;
}

void Ctrl_SendEvent(CtrlModeHandler_t*  obj, eventHandler_t* evHandler)
{
  if(evHandler && obj)
  {
    eventData_t event;
    event.eventNumber = eNewCtrlMode;
    event.data = obj;
    Event_Send(evHandler, event);
  }
}

FMode_t Ctrl_GetEventData(eventData_t* data)
{
  if(data && data->data)
  {
    CtrlModeHandler_t* ControlModeHanler = (CtrlModeHandler_t*)data->data;
    CtrlMode_t ctrlMode = Ctrl_GetCurrentMode(ControlModeHanler);
    return ctrlMode;
  }
  return Control_mode_not_available;
}

