/*
 * led.c
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

#include "led.h"
#include <gpio.h>
#include <pio.h>
#include "arduino_due_x.h"
#include "led_interface.h"

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
