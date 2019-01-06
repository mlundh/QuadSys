/*
 * setpoint_generator.c
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

#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"

#include "SetpointHandler/inc/setpoint_handler.h"

void SPHandl_TimerCallback( TimerHandle_t pxTimer );

struct SetpointData
{
  state_data_t setpoint;
  uint8_t prio;
};
typedef struct SetpointData SetpointData_t;

struct SpObj
{
  QueueHandle_t xQueue_Sp_Handl;
  TimerHandle_t xTimer;

};

const static uint8_t timerBlockTimeMs = 1;

/*Display queue*/
#define SP_HANDL_QUEUE_LENGTH      (1)
#define SP_HANDL_QUEUE_ITEM_SIZE         (sizeof(SetpointData_t))



SpHandler_t *SpHandl_Create()
{
  SpHandler_t* obj = pvPortMalloc(sizeof(SpHandler_t));
  obj->xQueue_Sp_Handl = xQueueCreate( SP_HANDL_QUEUE_LENGTH, SP_HANDL_QUEUE_ITEM_SIZE );
  obj->xTimer = xTimerCreate("Timer", 1, pdFALSE, ( void * ) obj, SPHandl_TimerCallback);

  if(!obj || ! obj->xQueue_Sp_Handl)
  {
    return NULL;
  }
  return obj;
}

uint8_t SpHandl_init(SpHandler_t *obj)
{
  return 1;
}

uint8_t SpHandl_SetSetpoint(SpHandler_t* obj, state_data_t* setpoint, uint8_t valid_for_ms, uint8_t prio )
{
  uint8_t result = 1;
  SetpointData_t setpointdata = {
      .setpoint = *setpoint, // Shallow copy is all that is needed!
      .prio = prio};

  SetpointData_t inQueue = {{{0}}};
  //if there is a message in the queue check if it has a higher prio then the current item.
  if(uxQueueMessagesWaiting(obj->xQueue_Sp_Handl))
  {
    if (xQueuePeek(obj->xQueue_Sp_Handl, &inQueue, timerBlockTimeMs/ portTICK_PERIOD_MS) != pdPASS )
    {
      return 0;
    }
    else
    {
      // Only let a setpoint with a equal or higher priority overwrite an active setpoint.
      if(inQueue.prio > setpointdata.prio)
      {
        return 2; // Valid to be refused to submit setpoint if the active setpoint has a higher priority.
      }
    }
  }

  //Update the period of the timer.
  if( xTimerChangePeriod( obj->xTimer, valid_for_ms / portTICK_PERIOD_MS, timerBlockTimeMs / portTICK_PERIOD_MS ) != pdPASS )
  {
    //Clear the queue, we are not able to set a correct timer value!
    xQueueReset(obj->xQueue_Sp_Handl);
    return 0;
  }
  // Reset the timer.
  if( xTimerReset( obj->xTimer, timerBlockTimeMs / portTICK_PERIOD_MS ) != pdPASS )
  {
    //Clear the queue, we are not able to set a correct timer value!
    xQueueReset(obj->xQueue_Sp_Handl);
    return 0;
  }
  BaseType_t write_result = xQueueOverwrite( obj->xQueue_Sp_Handl, &setpointdata );
  if(write_result != pdTRUE)
  {
    result &= 0; // Should never happen.
  }
  return result;
}

uint8_t SpHandl_GetSetpoint(SpHandler_t *obj, state_data_t *setpoint, uint8_t timeout_ms )
{
  if(!uxQueueMessagesWaiting(obj->xQueue_Sp_Handl))
  {
    return 0;
  }
  // Peek the queue, the timer callback will remove the item from the queue when it expires.
  if (xQueuePeek(obj->xQueue_Sp_Handl, setpoint, timeout_ms/ portTICK_PERIOD_MS) == pdPASS )
  {
    return 1;
  }
  return 0;
}

void SPHandl_TimerCallback( TimerHandle_t pxTimer )
{
  SpHandler_t *obj = ( SpHandler_t * ) pvTimerGetTimerID( pxTimer );
  xQueueReset(obj->xQueue_Sp_Handl);
  //TODO write error message!
}

