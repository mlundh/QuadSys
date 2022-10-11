/*
 * ArduinoDue_Gpio.c
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

#include "HAL/QuadFC/QuadFC_Gpio.h"
#include <gpio.h>
#include <pio.h>
#include <board.h>

uint8_t leds[] = {
    PIN_31_GPIO,
    PIN_33_GPIO,
    PIN_35_GPIO,
    PIN_37_GPIO,
    PIN_39_GPIO,
    PIN_41_GPIO
};
uint8_t pins[] = {
};
const static int nrPins = sizeof(pins)/sizeof(pins[0]);
const static int nrLeds = sizeof(leds)/sizeof(leds[0]);

void Gpio_PinReset(GpioName_t pin)
{
  if(pin < nrPins)
  {
    gpio_set_pin_low( pins[pin] );
  }
}
void Gpio_PinSet(GpioName_t pin)
{
  if(pin < nrPins)
  {
    gpio_set_pin_high( pins[pin] );
  }
}
void Gpio_TogglePin(GpioName_t pin)
{
  if(pin < nrPins)
  {
    gpio_toggle_pin( pins[pin] );
  }
}

void Led_On(LedName_t led)
{
  if(led < nrLeds)
  {
    gpio_set_pin_high( leds[led] );
  }
}
void Led_Off(LedName_t led)
{
  if(led < nrLeds)
  {
    gpio_set_pin_low( leds[led] );
  }
}
void Led_Toggle(LedName_t led)
{
  if(led < nrLeds)
  {
    gpio_toggle_pin( leds[led] );
  }
}
