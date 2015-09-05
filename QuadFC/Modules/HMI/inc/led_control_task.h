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

/**
 * Available led states.
 */
typedef enum LED_control
{
  // fc state.
  led_initializing = 0,             //!< led_initializing
  led_disarmed = 1,                 //!< led_disarmed
  led_configure = 2,                //!< led_configure
  led_arming = 3,                   //!< led_arming
  led_armed = 4,                    //!< led_armed
  led_fault = 5,                    //!< led_fault
  led_state_not_available = 6,      //!< led_state_not_available

  // control system mode
  led_control_mode_rate = 10,
  led_control_mode_attitude = 11,

  //System errors
  led_error_int_overflow = 20,      //!< led_error_int_overflow
  led_error_alloc = 21,             //!< led_error_alloc
  led_main_blocked = 22,            //!< led_main_blocked

  //Com errors
  led_error_TWI = 30,               //!< led_error_TWI
  led_error_rc_link = 31,           //!< led_error_rc_link
  led_warning_lost_com_message = 32,//!< led_warning_lost_com_message

  //Clear leds.
  led_clear_error = 50,             //!< led_clear_error
} LED_control_t;

/**
 * Create the led control task. This is used in the "main" function only.
 */
void Led_CreateLedControlTask( void );

/**
 * Request that the leds should indicate a specific state.
 *
 * Some states might be mutual exclusive, then the last state will be
 * showm.
 *
 *
 * @param led_control New led state to be set.
 * @return            pdTrue if successful.
 */
BaseType_t Led_Set(LED_control_t led_control);

#endif /* LED_CONTROL_TASK_H_ */
