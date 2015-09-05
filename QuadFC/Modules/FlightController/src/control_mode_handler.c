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
#include "control_mode_handler.h"


#include "led_control_task.h"
/*Semaphore for access control.*/
SemaphoreHandle_t xMutex;

/**
 * Queue holding current mode. Implemented
 * as a queue to ensure thread safety.
 */
#define MODE_CURRENT_QUEUE_LENGTH      (1)
#define MODE_CURRENT_QUEUE_ITEM_SIZE        (sizeof(CtrlMode_t))
static  QueueHandle_t Mode_current_queue;


/**
 * Internal current mode variable. TODO move to init?
 */
static CtrlMode_t current_mode = Control_mode_rate;

static BaseType_t Ctrl_ModeChangeAllowed(CtrlMode_t mode_req);
static void Ctrl_UpdateLed();

/*Initialize queues and mutex used internally
 * by the mode handler*/
void Ctrl_InitModeHandler()
{
  Mode_current_queue = xQueueCreate( MODE_CURRENT_QUEUE_LENGTH,
      MODE_CURRENT_QUEUE_ITEM_SIZE );
  xMutex = xSemaphoreCreateMutex();


  if(!Mode_current_queue || !xMutex)
  {
    //TODO send error!
  }
  current_mode = Control_mode_rate;
  if(xQueueSendToBack(Mode_current_queue, &current_mode, 2) != pdPASS)
  {
    //TODO send error!
  }
  Ctrl_UpdateLed();

}

/**
 * Get the current mode.
 */
CtrlMode_t Ctrl_GetCurrentMode()
{
  CtrlMode_t mode = Control_mode_not_available;
  if( !xQueuePeek(Mode_current_queue, &mode, 0) )
  {
    //no mode, error!
  }
  return mode;
}

// TODO add fault text readable from remote.
BaseType_t Ctrl_FaultMode()
{
  if( xSemaphoreTake( xMutex, ( TickType_t )(10UL / portTICK_PERIOD_MS) ) == pdTRUE )
  {
    CtrlMode_t mode_req = Control_mode_not_available;
    xQueueOverwrite(Mode_current_queue, &mode_req);
    Ctrl_UpdateLed();
    xSemaphoreGive(xMutex);
  }
  return pdTRUE;

}

BaseType_t Ctrl_ChangeMode(CtrlMode_t mode_req)
{
  BaseType_t result = pdFALSE;
  if( xSemaphoreTake( xMutex, ( TickType_t )(2UL / portTICK_PERIOD_MS) ) == pdTRUE )
  {
    if(Ctrl_ModeChangeAllowed(mode_req) == pdTRUE)
    {
      result = xQueueOverwrite(Mode_current_queue, &mode_req);
      Ctrl_UpdateLed();
    }
    xSemaphoreGive(xMutex);
  }
  /*Could not obtain mutex or change not allowed.*/
  return result;
}


static BaseType_t Ctrl_ModeChangeAllowed(CtrlMode_t mode_req)
{
  switch ( Ctrl_GetCurrentMode() )
  {
  case Control_mode_rate:
    if(    (mode_req == Control_mode_attitude) )
    {
      return pdTRUE;
    }
    break;
  case Control_mode_attitude:
    if(    (mode_req == Control_mode_rate))
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

static void Ctrl_UpdateLed()
{

  switch ( Ctrl_GetCurrentMode() )
  {
  case Control_mode_rate:
    Led_Set(led_control_mode_rate);
    return;
    break;

  case Control_mode_attitude:
    Led_Set(led_control_mode_attitude);
    return;
    break;

  default:
    break;
  }
}


