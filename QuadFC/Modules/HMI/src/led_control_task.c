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

#include "led_control_task.h"
#include "led.h"
#include "led_interface.h"
#include "stdint.h"

#include "task.h"
#include "queue.h"

/*LED queue*/
#define LED_QUEUE_LENGTH      (5)
#define LED_QUEUE_ITEM_SIZE        (sizeof(uint8_t))
QueueHandle_t xQueue_led;

/**
 * The control task.
 * @param pvParameters
 */
void Led_ControlTask( void *pvParameters );

void Led_CreateLedControlTask( void )
{

  /*Create the task*/
  xQueue_led = xQueueCreate( LED_QUEUE_LENGTH, LED_QUEUE_ITEM_SIZE );
  if(!xQueue_led)
  {
    for(;;); //Error!
  }
  portBASE_TYPE create_result;
  create_result = xTaskCreate( Led_ControlTask,  /* The function that implements the task.  */
      (const char *const) "Led_Ctrl",      /* The name of the task. This is not used by the kernel, only aids in debugging*/
      500,                                        /* The stack size for the task*/
      NULL,                                       /* No input parameters*/
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
  /*led_control_task execute at 10Hz*/
  static const TickType_t xPeriod = (20UL / portTICK_PERIOD_MS);

  static uint8_t ctrl = 0;

  static uint8_t mode_state_led_1 = 0;
  static uint32_t counter_state_led_1 = 0;
  static uint8_t on_off_state_led_1 = 0;

  static uint8_t mode_state_led_2 = 0;
  static uint32_t counter_state_led_2 = 0;
  static uint8_t on_off_state_led_2 = 0;

  static uint8_t mode_error_led_1 = 0;
  static uint32_t counter_error_led_1 = 0;
  static uint8_t on_off_error_led_1 = 0;

  static uint8_t mode_error_led_2 = 0;
  static uint32_t counter_error_led_2 = 0;
  static uint8_t on_off_error_led_2 = 0;

  unsigned portBASE_TYPE xLastWakeTime = xTaskGetTickCount();
  for ( ;; )
  {

    xQueueReceive( xQueue_led, &ctrl, 0 );

    switch ( ctrl )
    {

    case led_initializing:
      mode_state_led_1 = led_blink_fast;
      break;

    case led_disarmed:
      mode_state_led_1 = led_double_blink;
      break;

    case led_configure:
      mode_state_led_1 = led_const_on;
      break;

    case led_arming:
      mode_state_led_1 = led_blink_fast;
      break;

    case led_armed:
      mode_state_led_1 = led_blink_slow;
      break;

    case led_fault:
      mode_state_led_1 = led_const_on;
      mode_state_led_2 = led_const_on;
      break;

    case led_control_mode_rate:
      mode_state_led_2 = led_double_blink;
      break;

    case led_control_mode_attitude:
      mode_state_led_2 = led_blink_slow;
      break;

    case led_error_alloc:
      mode_error_led_1 = led_double_blink;
      break;

    case led_error_rc_link:
      mode_error_led_2 = led_const_on;
      break;

    case led_error_TWI:
      mode_error_led_2 = led_blink_slow;
      break;

    case led_warning_lost_com_message:
      mode_error_led_2 = led_const_on;
      break;

    case led_clear_error:
      mode_error_led_1 = led_off;
      mode_error_led_2 = led_off;
      break;

    default:
      break;
    }

    led_handler( mode_state_led_1, PIN_35_GPIO, &counter_state_led_1, &on_off_state_led_1 );
    led_handler( mode_state_led_2, PIN_37_GPIO, &counter_state_led_2, &on_off_state_led_2 );
    led_handler( mode_error_led_1, PIN_39_GPIO, &counter_error_led_1, &on_off_error_led_1 );
    led_handler( mode_error_led_2, PIN_41_GPIO, &counter_error_led_2, &on_off_error_led_2 );

    //TODO add more led pins!

    vTaskDelayUntil( &xLastWakeTime, xPeriod );
  }

}



BaseType_t Led_Set(LED_control_t led_control)
{
  return xQueueSendToBack( xQueue_led, &led_control, 0 );
  //TODO force send if not successful?
}


