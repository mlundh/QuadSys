/*
 * led_control_task.c
 *
 * Copyright (C) 2014 martin
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

#include "../inc/led_control_task.h"
#include <stdint.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "EventHandler/inc/event_handler.h"
#include "FlightModeHandler/inc/flight_mode_handler.h"
#include "FlightController/inc/control_mode_handler.h"
#include "HAL/QuadFC/QuadFC_Gpio.h"

#define NR_LEDS (6)
#define LED_TASK_DELAY (20UL / portTICK_PERIOD_MS)

typedef enum led_mode
{
  led_off,
  led_blink_fast,
  led_blink_slow,
  led_double_blink,
  led_const_on,
} led_mode_t;

typedef enum ledBlinkState
{
  first,
  second,
  third,
  fourth,
} ledBlinkState;

typedef struct ledState
{
  led_mode_t    mode;
  uint32_t      counter;
  ledBlinkState currentState;
}ledState_t;



typedef struct ledTaskParams
{
  ledState_t      ledState[NR_LEDS];
  eventHandler_t* evHandler;
}ledTaskParams;


/**
 * The control task.
 * @param pvParameters
 */
void Led_ControlTask( void *pvParameters );


void Led_update( ledState_t* ledState, GpioName_t pin);

uint8_t Led_HandleFlightMode(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t Led_HandleCtrltMode(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t Led_HandlePeripheralError(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);

void Led_toggleLed(ledState_t* ledState, GpioName_t pin);

void Led_resetLeds(ledTaskParams * param);

void Led_CreateLedControlTask( QueueHandle_t eventMaster )
{

  /*Create the task*/
  ledTaskParams* ledParams = pvPortMalloc(sizeof(ledTaskParams));
  if(!ledParams)
  {
    for(;;); //Error!
  }
  ledParams->evHandler = Event_CreateHandler(eventMaster, 0);
  if(!ledParams->evHandler)
  {
    for(;;); //Error!
  }

  ledState_t tmp = {0};
  for(int i = 0; i < NR_LEDS; i++)
  {
    ledParams->ledState[i] = tmp;
  }

  Event_RegisterCallback(ledParams->evHandler, eNewFlightMode    ,Led_HandleFlightMode, ledParams);
  Event_RegisterCallback(ledParams->evHandler, eNewCtrlMode      ,Led_HandleCtrltMode, ledParams);
  Event_RegisterCallback(ledParams->evHandler, ePeripheralError  ,Led_HandlePeripheralError, ledParams);


  portBASE_TYPE create_result;
  create_result = xTaskCreate( Led_ControlTask,   /* The function that implements the task.  */
      (const char *const) "Led_Ctrl",             /* The name of the task. This is not used by the kernel, only aids in debugging*/
      500,                                        /* The stack size for the task*/
      ledParams,                                  /* Send the task parameter to the task.*/
      configMAX_PRIORITIES-4,                     /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                     /* Handle to the task. Not used here and therefore NULL*/

  if ( create_result != pdTRUE )
  {
    /*Error - Could not create task. TODO handle this error*/
    for ( ;; )
    {

    }
  }

}

void Led_ControlTask( void *pvParameters )
{
  ledTaskParams * param = (ledTaskParams*)(pvParameters);

  Event_StartInitialize(param->evHandler);
  Event_EndInitialize(param->evHandler);



  unsigned portBASE_TYPE xLastWakeTime = xTaskGetTickCount();
  for ( ;; )
  {
    vTaskDelayUntil( &xLastWakeTime, LED_TASK_DELAY );
    while(Event_Receive(param->evHandler, 0) == 1)
    {}
    for(int i = 0; i < NR_LEDS; i++)
    {
      if(i != ledGreen1) // ledGreen1 is heart beat, controlled by main task.
      {
        Led_update(&param->ledState[i], i);
      }
    }
  }

}

void Led_update( ledState_t* ledState, GpioName_t pin)
{
  ledState->counter++;
  switch ( ledState->mode )
  {
  case led_off:
    Gpio_SetPinLow( pin );
    ledState->counter = 0;
    break;

  case led_const_on:
    Gpio_SetPinHigh( pin );
    ledState->counter = 0;
    break;

  case led_blink_fast:
    if ( ledState->counter >= 2 )
    {
      Led_toggleLed( ledState, pin );
      ledState->counter = 0;
    }
    break;
  case led_blink_slow:

    if ( ledState->counter >= 10 )
    {
      Led_toggleLed( ledState, pin );
      ledState->counter = 0;
    }
    break;

  case led_double_blink:
    if ( ledState->currentState == first )
    {
      if ( ledState->counter >= 20 )
      {
        Gpio_SetPinHigh( pin );
        ledState->counter = 0;
        ledState->currentState = second;
      }
    }
    else if ( ledState->currentState == second )
    {
      if ( ledState->counter >= 4 )
      {
        Gpio_SetPinLow( pin );
        ledState->counter = 0;
        ledState->currentState = third;
      }
    }
    else if ( ledState->currentState == third )
    {
      if ( ledState->counter >= 2 )
      {
        Gpio_SetPinHigh( pin );
        ledState->counter = 0;
        ledState->currentState = fourth;
      }
    }
    else
    {
      if ( ledState->counter >= fourth )
      {
        Gpio_SetPinLow( pin );
        ledState->counter = 0;
        ledState->currentState = first;
      }
    }
    break;

  default:
    break;
  }
}

void Led_toggleLed(ledState_t* ledState, GpioName_t pin )
{

  if ( ledState->currentState != first )
  {
    Gpio_SetPinHigh( pin );
    ledState->currentState = first;
  }
  else
  {
    Gpio_SetPinLow( pin );
    ledState->currentState = second;
  }
}

uint8_t Led_HandleFlightMode(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
  ledTaskParams * param = (ledTaskParams*)(taskParam);
  FlightMode_t flightMode = Msg_FlightModeGetMode(data);

  switch(flightMode)
  {
  case fmode_init:
    param->ledState[ledYellow1].mode = led_blink_fast;
    break;
  case fmode_disarmed:
    param->ledState[ledRed1].mode    = led_off;
    param->ledState[ledRed2].mode    = led_off;
    param->ledState[ledYellow1].mode = led_blink_slow;
    break;
  case fmode_config:
    param->ledState[ledYellow1].mode = led_const_on;
    break;
  case fmode_arming:
    param->ledState[ledYellow1].mode = led_blink_fast;
    break;
  case fmode_armed:
    param->ledState[ledYellow1].mode = led_double_blink;
    break;
  case fmode_disarming:
    param->ledState[ledYellow1].mode = led_blink_fast;
    break;
  case fmode_fault:
    param->ledState[ledRed1].mode    = led_const_on;
    param->ledState[ledRed2].mode    = led_const_on;
    param->ledState[ledYellow1].mode = led_const_on;

    break;
  case fmode_not_available:
    break;
  default:
    break;
  }
  return 1;
}
uint8_t Led_HandleCtrltMode(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
  ledTaskParams * param = (ledTaskParams*)(taskParam);
  CtrlMode_t flightState = Ctrl_GetEventData(data);
  switch(flightState)
  {
  case Control_mode_not_available:
    param->ledState[ledYellow2].mode = led_off;
    break;
  case Control_mode_rate:
    param->ledState[ledYellow2].mode = led_blink_fast;
    break;
  case Control_mode_attitude:
    param->ledState[ledYellow2].mode = led_blink_slow;
    break;
  default:
    break;
  }
  return 0;
}

uint8_t Led_HandlePeripheralError(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
  ledTaskParams * param = (ledTaskParams*)(taskParam);
  param->ledState[ledRed2].mode    = led_const_on;
  return 0;
}

void Led_resetLeds(ledTaskParams * param)
{
  param->ledState[ledRed1].mode    = led_off;
  param->ledState[ledRed2].mode    = led_off;
  param->ledState[ledYellow1].mode = led_off;
  param->ledState[ledYellow2].mode = led_off;
  param->ledState[ledGreen1].mode  = led_off;
  param->ledState[ledGreen2].mode  = led_off;
}
