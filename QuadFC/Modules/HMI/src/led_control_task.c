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
#include <gpio.h>
#include <pio.h>
#include "arduino_due_x.h"

void create_led_control_task( void )
{

  /*Create the task*/
  portBASE_TYPE create_result;
  create_result = xTaskCreate( led_control_task,  /* The function that implements the task.  */
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

void led_control_task( void *pvParameters )
{
  /*led_control_task execute at 10Hz*/
  static const TickType_t xPeriod = (20UL / portTICK_PERIOD_MS);

  uint8_t ctrl = 0;

  uint8_t mode_state_led_1 = 0;
  uint32_t counter_state_led_1 = 0;
  uint8_t on_off_state_led_1 = 0;

  uint8_t mode_state_led_2 = 0;
  uint32_t counter_state_led_2 = 0;
  uint8_t on_off_state_led_2 = 0;

  uint8_t mode_error_led_1 = 0;
  uint32_t counter_error_led_1 = 0;
  uint8_t on_off_error_led_1 = 0;

  uint8_t mode_error_led_2 = 0;
  uint32_t counter_error_led_2 = 0;
  uint8_t on_off_error_led_2 = 0;

  unsigned portBASE_TYPE xLastWakeTime = xTaskGetTickCount();
  for ( ;; )
  {

    xQueueReceive( xQueue_led, &ctrl, 0 );

    switch ( ctrl )
    {
    case fc_disarmed_led:
      mode_state_led_1 = led_double_blink;
      mode_state_led_2 = led_off;
      break;

    case fc_arming_led:
      mode_state_led_1 = led_blink_fast;
      break;

    case fc_armed_rate_mode_led:
      mode_state_led_1 = led_blink_slow;
      break;

    case fc_armed_angle_mode_led:
      mode_state_led_1 = 0;
      break;

    case fc_configure_led:
      mode_state_led_1 = led_const_on;
      break;

    case fc_initializing_led:
      mode_state_led_2 = led_blink_fast;
      break;

    case error_int_overflow_led:
      mode_error_led_1 = led_blink_fast;
      break;

    case error_alloc_led:
      mode_error_led_1 = led_double_blink;
      break;

    case error_rc_link_led:
      mode_error_led_1 = led_const_on;
      break;

    case error_TWI_led:
      mode_error_led_2 = led_blink_slow;
      break;

    case warning_lost_com_message:
      mode_error_led_2 = led_const_on;
      break;

    case clear_error_led:
      mode_error_led_1 = led_off;
      break;
      
    case clear_error_led2:
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

void led_handler( uint8_t mode, uint8_t pin, uint32_t *counter, uint8_t *on_off )
{
  *counter = *counter + 1;
  switch ( mode )
  {
  case led_off:
    gpio_set_pin_low( pin );
    *counter = 0;
    break;
  case led_const_on:
    gpio_set_pin_high( pin );
    *counter = 0;
    break;
  case led_blink_fast:
    if ( *counter >= 2 )
    {
      toggle_led( pin, on_off );
      *counter = 0;
    }
    break;
  case led_blink_slow:

    if ( *counter >= 10 )
    {
      toggle_led( pin, on_off );
      *counter = 0;
    }
    break;
  case led_double_blink:
    if ( *on_off == 0 )
    {
      if ( *counter >= 20 )
      {
        gpio_set_pin_high( pin );
        *counter = 0;
        *on_off = 1;
      }
    }
    else if ( *on_off == 1 )
    {
      if ( *counter >= 4 )
      {
        gpio_set_pin_low( pin );
        *counter = 0;
        *on_off = 2;
      }
    }
    else if ( *on_off == 2 )
    {
      if ( *counter >= 2 )
      {
        gpio_set_pin_high( pin );
        *counter = 0;
        *on_off = 3;
      }
    }
    else
    {
      if ( *counter >= 4 )
      {
        gpio_set_pin_low( pin );
        *counter = 0;
        *on_off = 0;
      }
    }
    break;
  default:
    break;

  }
}

void toggle_led( uint8_t pin, uint8_t* on_off )
{

  if ( *on_off != 0 )
  {
    gpio_set_pin_high( pin );
    *on_off = 0;
  }
  else
  {
    gpio_set_pin_low( pin );
    *on_off = 1;
  }
}

