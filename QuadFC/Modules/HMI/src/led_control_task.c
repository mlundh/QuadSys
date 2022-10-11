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
#include "ControlModeHandler/inc/control_mode_handler.h"
#include "Messages/inc/Msg_CtrlMode.h"
#include "HAL/QuadFC/QuadFC_Gpio.h"
#include "Messages/inc/Msg_ValidSp.h"

#define NR_LEDS (6)
#define LED_TASK_DELAY (20UL / portTICK_PERIOD_MS)

typedef enum LedName
{
  ledSetPoint,
  ledStatus,
  ledmode,
  ledError,
  ledFatal,
  ledNrGpio,
}led_control_t;

typedef enum led_mode
{
  led_Off,
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
  GpioName_t    gpio;
  led_mode_t    mode;
  uint32_t      counter;
  ledBlinkState currentState;
}ledState_t;



typedef struct ledTaskParams
{
  eventHandler_t* evHandler;
}ledTaskParams;

static ledState_t ledState[] = 
{
  {LED_SETPOINT,  led_Off, 0, first},
  {LED_STATUS,    led_Off, 0, first},
  {LED_CTRL_MODE, led_Off, 0, first},
  {LED_ERROR,     led_Off, 0, first},
  {LED_FATAL,     led_Off, 0, first},
};

/**
 * The control task.
 * @param pvParameters
 */
void Led_ControlTask( void *pvParameters );


void Led_update( ledState_t* ledState);

uint8_t Led_HandleFlightMode(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t Led_HandleCtrltMode(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t Led_HandlePeripheralError(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t Led_HandleValidSp(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);

void Led_toggleLed(ledState_t* ledState);

void Led_resetLeds();

void Led_CreateLedControlTask(eventHandler_t* evHandler )
{

  /*Create the task*/
  ledTaskParams* ledParams = pvPortMalloc(sizeof(ledTaskParams));
  if(!ledParams)
  {
    for(;;); //Error!
  }
  ledParams->evHandler = evHandler;
  if(!ledParams->evHandler)
  {
    for(;;); //Error!
  }
  Gpio_Init(LED_SETPOINT, Gpio_OutputOpenDrain, Gpio_NoPull);
  Gpio_Init(LED_STATUS, Gpio_OutputOpenDrain, Gpio_NoPull);
  Gpio_Init(LED_CTRL_MODE, Gpio_OutputOpenDrain, Gpio_NoPull);
  Gpio_Init(LED_ERROR, Gpio_OutputOpenDrain, Gpio_NoPull);
  Gpio_Init(LED_FATAL, Gpio_OutputOpenDrain, Gpio_NoPull);
  Led_resetLeds();
  
  Event_RegisterCallback(ledParams->evHandler, Msg_FlightMode_e,    Led_HandleFlightMode,      ledParams);
  Event_RegisterCallback(ledParams->evHandler, Msg_CtrlMode_e,      Led_HandleCtrltMode,       ledParams);
  Event_RegisterCallback(ledParams->evHandler, Msg_Error_e,         Led_HandlePeripheralError, ledParams);
  Event_RegisterCallback(ledParams->evHandler, Msg_ValidSp_e,       Led_HandleValidSp,         ledParams);


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


  unsigned portBASE_TYPE xLastWakeTime = xTaskGetTickCount();
  for ( ;; )
  {
    vTaskDelayUntil( &xLastWakeTime, LED_TASK_DELAY );
    while(Event_Receive(param->evHandler, 0) == 1)
    {}
    for(int i = 0; i < ledNrGpio; i++)
    {

        Led_update(&ledState[i]);
    }
  }

}

void Led_update( ledState_t* state)
{
  state->counter++;
  switch ( state->mode )
  {
  case led_Off:
    Gpio_PinSet( state->gpio );
    state->counter = 0;
    break;

  case led_const_on:
    Gpio_PinReset( state->gpio );
    state->counter = 0;
    break;

  case led_blink_fast:
    if ( state->counter >= 2 )
    {
      Led_toggleLed( state );
      state->counter = 0;
    }
    break;
  case led_blink_slow:

    if ( state->counter >= 10 )
    {
      Led_toggleLed( state );
      state->counter = 0;
    }
    break;

  case led_double_blink:
    if ( state->currentState == first )
    {
      if ( state->counter >= 20 )
      {
        Gpio_PinReset( state->gpio );
        state->counter = 0;
        state->currentState = second;
      }
    }
    else if ( state->currentState == second )
    {
      if ( state->counter >= 4 )
      {
        Gpio_PinSet( state->gpio );
        state->counter = 0;
        state->currentState = third;
      }
    }
    else if ( state->currentState == third )
    {
      if ( state->counter >= 2 )
      {
        Gpio_PinReset( state->gpio );
        state->counter = 0;
        state->currentState = fourth;
      }
    }
    else
    {
      if ( state->counter >= fourth )
      {
        Gpio_PinSet( state->gpio );
        state->counter = 0;
        state->currentState = first;
      }
    }
    break;

  default:
    break;
  }
}

void Led_toggleLed(ledState_t* state )
{

  if ( state->currentState != first )
  {
    Gpio_PinReset( state->gpio );
    state->currentState = first;
  }
  else
  {
    Gpio_PinSet( state->gpio );
    state->currentState = second;
  }
}

uint8_t Led_HandleFlightMode(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
  FlightMode_t flightMode = Msg_FlightModeGetMode(data);

  switch(flightMode)
  {
  case fmode_init:
    ledState[ledStatus].mode = led_blink_slow;
    break;
  case fmode_disarmed:
    ledState[ledError].mode    = led_Off;
    ledState[ledFatal].mode    = led_Off;
    ledState[ledStatus].mode   = led_Off;
    break;
  case fmode_config:
    ledState[ledStatus].mode = led_double_blink;
    break;
  case fmode_arming:
    ledState[ledStatus].mode = led_blink_fast;
    break;
  case fmode_armed:
    ledState[ledStatus].mode = led_const_on;
    break;
  case fmode_disarming:
    ledState[ledStatus].mode = led_blink_fast;
    break;
  case fmode_fault:
    ledState[ledError].mode    = led_const_on;
    ledState[ledFatal].mode    = led_const_on;
    ledState[ledStatus].mode   = led_const_on;

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
  CtrlMode_t flightState = Msg_CtrlModeGetMode(data);
  switch(flightState)
  {
  case Control_mode_not_available:
    ledState[ledmode].mode = led_Off;
    break;
  case Control_mode_rate:
    ledState[ledmode].mode = led_const_on;
    break;
  case Control_mode_attitude:
    ledState[ledmode].mode = led_blink_slow;
    break;
  default:
    break;
  }
  return 0;
}

uint8_t Led_HandlePeripheralError(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
  ledState[ledFatal].mode    = led_const_on;
  return 0;
}

void Led_resetLeds()
{
  ledState[ledError].mode = led_Off;
  ledState[ledFatal].mode = led_Off;
  ledState[ledStatus].mode = led_Off;
  ledState[ledmode].mode = led_Off;
  ledState[ledSetPoint].mode = led_Off;
}

uint8_t Led_HandleValidSp(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
  uint32_t validSp = Msg_ValidSpGetValid(data);

  if(validSp)
  {
    ledState[ledSetPoint].mode = led_const_on;
  }
  else
  {
    ledState[ledSetPoint].mode = led_Off;
  }
  
  return 1;
}