/*
 * Titan_Gpio.c
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
#include "cubeInit.h"

#define NR_PINS (26)

uint16_t pins[] = {
    HEARTBEAT_Pin,
    LED_SETPOINT_Pin,
    LED_STATUS_Pin,
    LED_CTRL_MODE_Pin,
    LED_ERROR_Pin,
    LED_FATAL_Pin,

    IO_1_Pin,
    IO_2_Pin,
    IO_3_Pin,
    IO_4_Pin,
    IO_5_Pin,
    IO_6_Pin,
    IO_7_Pin,
    IO_8_Pin,
    IO_9_Pin,
    IO_10_Pin,
    IO_11_Pin,
    IO_12_Pin,
    IO_13_Pin,
    IO_14_Pin,
    IO_15_Pin,
    //IO_16_Pin,

    RC_1_PWR_CTRL_Pin,
    RC_2_PWR_CTRL_Pin,

    ADC_BAT_Pin,
    ADC_1_Pin,
    ADC_2_Pin,
    ADC_3_Pin,

};

GPIO_TypeDef* pinPorts[] = {
    HEARTBEAT_GPIO_Port,
    LED_SETPOINT_GPIO_Port,
    LED_STATUS_GPIO_Port,
    LED_CTRL_MODE_GPIO_Port,
    LED_ERROR_GPIO_Port,
    LED_FATAL_GPIO_Port,

    IO_1_GPIO_Port,
    IO_2_GPIO_Port,
    IO_3_GPIO_Port,
    IO_4_GPIO_Port,
    IO_5_GPIO_Port,
    IO_6_GPIO_Port,
    IO_7_GPIO_Port,
    IO_8_GPIO_Port,
    IO_9_GPIO_Port,
    IO_10_GPIO_Port,
    IO_11_GPIO_Port,
    IO_12_GPIO_Port,
    IO_13_GPIO_Port,
    IO_14_GPIO_Port,
    IO_15_GPIO_Port,
    //IO_16_GPIO_Port,

    RC_1_PWR_CTRL_GPIO_Port,
    RC_2_PWR_CTRL_GPIO_Port,

    ADC_BAT_GPIO_Port,
    ADC_1_GPIO_Port,
    ADC_2_GPIO_Port,
    ADC_3_GPIO_Port,
    

};
void Gpio_SetPinLow(GpioName_t pin)
{
  if(pin < NR_PINS)
  {
    HAL_GPIO_WritePin( pinPorts[pin], pins[pin], GPIO_PIN_RESET);
  }
}
void Gpio_SetPinHigh(GpioName_t pin)
{
  if(pin < NR_PINS)
  {
    HAL_GPIO_WritePin( pinPorts[pin], pins[pin], GPIO_PIN_SET);
  }
}
void Gpio_TogglePin(GpioName_t pin)
{
  if(pin < NR_PINS)
  {
    HAL_GPIO_TogglePin( pinPorts[pin], pins[pin] );
  }
}
