/*
 * state_handler.c
 *
 * Copyright (C) 2015 martin
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

#include "state_handler.h"
#include "led_control_task.h"
/*Semaphore for access control.*/
SemaphoreHandle_t xMutex;

/**
 * Queue holding current state. Implemented
 * as a queue to ensure thread safety.
 */
#define STATE_CURRENT_QUEUE_LENGTH      (1)
#define STATE_CURRENT_QUEUE_ITEM_SIZE        (sizeof(state_t))
static  QueueHandle_t State_current_queue;

/**
 * State lock queue. Use this to prevent state changes.
 * Implemented as a queue to ensure thread safety.
 */
#define STATE_LOCK_QUEUE_LENGTH      (1)
#define STATE_LOCK_QUEUE_ITEM_SIZE        (sizeof(uint8_t))
static  QueueHandle_t State_lock_queue;

/**
 * Internal current state variable. TODO move to init?
 */
static state_t current_state = state_init;

static BaseType_t State_ChangeAllowed(state_t state_req);
static void State_UpdateLed();

/*Initialize queues and mutex used internally
 * by the state handler*/
void State_InitStateHandler()
{
  State_current_queue = xQueueCreate( STATE_CURRENT_QUEUE_LENGTH,
      STATE_CURRENT_QUEUE_ITEM_SIZE );
  State_lock_queue = xQueueCreate(STATE_LOCK_QUEUE_LENGTH, STATE_LOCK_QUEUE_ITEM_SIZE);
  xMutex = xSemaphoreCreateMutex();


  if( !State_lock_queue
      || !State_lock_queue
      || !xMutex)
  {
    //TODO send error!
  }
  uint8_t lock = 0;
  current_state = state_init;

  if(( xQueueSendToBack(State_lock_queue, &lock, 2) != pdPASS )
      || (xQueueSendToBack(State_current_queue, &current_state, 2) != pdPASS))
  {
    //TODO send error!
  }

  State_UpdateLed();

}

/**
 * Get the current state.
 */
state_t State_GetCurrent()
{
  state_t state = state_not_available;
  if( !xQueuePeek(State_current_queue, &state, 0) )
  {
    //no state, error!
  }
  return state;
}

BaseType_t State_Lock()
{
  uint8_t lock = 1;
  xQueueOverwrite(State_lock_queue, &lock);
  return pdTRUE;
}

BaseType_t State_Unlock()
{
  uint8_t lock = 0;
  xQueueOverwrite(State_lock_queue, &lock);
  return pdTRUE;
}

// TODO add fault text readable from remote.
BaseType_t State_Fault()
{
  state_t state_req = state_fault;
  //State_Lock();
  xQueueOverwrite(State_current_queue, &state_req);
  State_UpdateLed();
  return pdTRUE;

}

BaseType_t State_Change(state_t state_req)
{
  BaseType_t result = pdFALSE;
  if( xSemaphoreTake( xMutex, ( TickType_t )(2UL / portTICK_PERIOD_MS) ) == pdTRUE )
  {
    if(State_ChangeAllowed(state_req) == pdTRUE)
    {
      result = xQueueOverwrite(State_current_queue, &state_req);
      State_UpdateLed();
    }
    xSemaphoreGive(xMutex);
  }
  /*Could not obtain mutex or change not allowed.*/
  return result;
}


static BaseType_t State_ChangeAllowed(state_t state_req)
{
  uint8_t lock = 0;

  BaseType_t read_success = xQueuePeek(State_lock_queue, &lock, ( TickType_t )(2UL / portTICK_PERIOD_MS) );

  if(pdTRUE != read_success || lock != 0)
  {
    //Locked or timeout.
    return pdFALSE;
  }

  switch ( State_GetCurrent() )
  {
  case state_init:
    if(    (state_req == state_disarming) )
    {
      return pdTRUE;
    }
    break;
  case state_disarmed:
    if(    (state_req == state_config)
        || (state_req == state_init )
        || (state_req == state_armed )
        || (state_req == state_arming )
        || (state_req == state_fault ) )
    {
      return pdTRUE;
    }
    break;
  case state_config:
    if(    (state_req == state_disarming)
        || (state_req == state_fault ) )
    {
      return pdTRUE;
    }
    break;
  case state_arming:
    if(    (state_req == state_armed)
        || (state_req == state_disarming )
        || (state_req == state_fault ) )
    {
      return pdTRUE;
    }
    break;
  case state_armed:
    if(    (state_req == state_disarming)
        || (state_req == state_fault ) )
    {
      return pdTRUE;
    }
    break;
  case   state_disarming:
    if(    (state_req == state_disarmed)
        || (state_req == state_fault ) )
    {
      return pdTRUE;
    }
    break;
  case state_fault:
    if(    (state_req == state_disarming) )
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

static void State_UpdateLed()
{

  switch ( State_GetCurrent() )
  {
  case state_init:
    Led_Set(led_initializing);
    return;
    break;

  case state_disarmed:
    Led_Set(led_disarmed);
    return;
    break;

  case state_config:
    Led_Set(led_configure);
    return;
    break;

  case state_arming:
    Led_Set(led_arming);
    return;

    break;
  case state_armed:
    Led_Set(led_armed);
    return;
    break;

  case state_fault:
    Led_Set(led_fault);
    return;
    break;

  default:
    break;
  }
}
