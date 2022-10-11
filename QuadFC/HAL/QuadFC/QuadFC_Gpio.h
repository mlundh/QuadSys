/*
 * QuadFC_Gpio.h
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
#ifndef HAL_QUADFC_QUADFC_GPIO_H_
#define HAL_QUADFC_QUADFC_GPIO_H_
#include "Messages/inc/common_types.h"

typedef enum{
    Gpio_Rc1PwrCtrl,
    Gpio_Rc2PwrCtrl,
    Gpio_AdcBat,
    Gpio_Adc1,
    Gpio_Adc2,
    Gpio_Adc3,
    Gpio_Io1,
    Gpio_Io2,
    Gpio_Io3,
    Gpio_Io4,
    Gpio_Io5,
    Gpio_Io6,
    Gpio_Io7,
    Gpio_Io8,
    Gpio_Io9,
    Gpio_Io10,
    Gpio_Io11,
    Gpio_Io12,
    Gpio_Io13,
    Gpio_Io14,
    Gpio_Io15,
    Gpio_Pwm1,
    Gpio_Pwm2,
    Gpio_Pwm3,
    Gpio_Pwm4,
    HEARTBEAT,
    LED_SETPOINT,
    LED_STATUS,
    LED_CTRL_MODE,
    LED_ERROR,
    LED_FATAL,
    nrPins,
}GpioName_t;

typedef enum {
    Gpio_InputDigital,
    Gpio_InputAnalog,
    Gpio_InputInterruptRising,
    Gpio_InputInterruptFalling,
    Gpio_InputInterruptBoth,
    Gpio_OutputPullPush,
    Gpio_OutputOpenDrain,
} GpioMode_t;

typedef enum {
    Gpio_NoPull,
    Gpio_PullUp,
    Gpio_PullDown,
} Gpio_Pull_t;

void Gpio_Init(GpioName_t pin, GpioMode_t mode, Gpio_Pull_t pull);
void Gpio_PinReset(GpioName_t pin);
void Gpio_PinSet(GpioName_t pin);
void Gpio_TogglePin(GpioName_t pin);



#endif /* HAL_QUADFC_QUADFC_GPIO_H_ */
