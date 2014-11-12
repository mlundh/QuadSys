/*
 * led_control_task.h
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

#ifndef LED_CONTROL_TASK_H_
#define LED_CONTROL_TASK_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdint.h"

/*LED queue*/
#define LED_QUEUE_LENGTH			(5)
#define LED_QUEUE_ITEM_SIZE        (sizeof(uint8_t))
extern xQueueHandle xQueue_led;

typedef enum LED_control
{

  fc_disarmed_led = 1,
  fc_arming_led = 2,
  fc_armed_rate_mode_led = 3,
  fc_armed_angle_mode_led = 4,
  fc_configure_led = 5,

  fc_initializing_led = 10,

  error_int_overflow_led = 20,
  error_TWI_led = 21,
  error_alloc_led = 22,
  error_rc_link_led = 23,

  warning_lost_com_message = 30,

  clear_error_led = 40,
  clear_error_led2 = 41,
} LED_control_t;

typedef enum led_mode
{
  led_off = 0,
  led_blink_fast = 1,
  led_blink_slow = 2,
  led_double_blink = 3,
  led_const_on = 4,
} led_mode_t;

void create_led_control_task( void );
void led_control_task( void *pvParameters );
void led_handler( uint8_t mode, uint8_t pin, uint32_t *counter,
    uint8_t *on_off );
void toggle_led( uint8_t pin, uint8_t* on_off );
#endif /* LED_CONTROL_TASK_H_ */
