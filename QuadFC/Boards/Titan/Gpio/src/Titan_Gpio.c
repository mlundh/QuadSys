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

typedef struct titan_gpio_control
{
  uint16_t pin;
  GPIO_TypeDef *pinPort;
  uint8_t initalState;
} titan_gpio_control_t;

static titan_gpio_control_t pins[] = {
    {RC_1_PWR_CTRL_Pin, RC_1_PWR_CTRL_GPIO_Port, 0},
    {RC_2_PWR_CTRL_Pin, RC_2_PWR_CTRL_GPIO_Port, 0},
    {ADC_BAT_Pin, ADC_BAT_GPIO_Port, 0},
    {ADC_1_Pin, ADC_1_GPIO_Port, 0},
    {ADC_2_Pin, ADC_2_GPIO_Port, 0},
    {ADC_3_Pin, ADC_3_GPIO_Port, 0},
    {IO_1_Pin, IO_1_GPIO_Port, 0},
    {IO_2_Pin, IO_2_GPIO_Port, 0},
    {IO_3_Pin, IO_3_GPIO_Port, 0},
    {IO_4_Pin, IO_4_GPIO_Port, 0},
    {IO_5_Pin, IO_5_GPIO_Port, 0},
    {IO_6_Pin, IO_6_GPIO_Port, 0},
    {IO_7_Pin, IO_7_GPIO_Port, 0},
    {IO_8_Pin, IO_8_GPIO_Port, 0},
    {IO_9_Pin, IO_9_GPIO_Port, 0},
    {IO_10_Pin, IO_10_GPIO_Port, 0},
    {IO_11_Pin, IO_11_GPIO_Port, 0},
    {IO_12_Pin, IO_12_GPIO_Port, 0},
    {IO_13_Pin, IO_13_GPIO_Port, 0},
    {IO_14_Pin, IO_14_GPIO_Port, 0},
    {IO_15_Pin, IO_15_GPIO_Port, 0},
    {PWM_1_Pin, PWM_1_GPIO_Port, 0},
    {PWM_2_Pin, PWM_2_GPIO_Port, 0},
    {PWM_3_Pin, PWM_3_GPIO_Port, 0},
    {PWM_4_Pin, PWM_4_GPIO_Port, 0},
    {HEARTBEAT_Pin, HEARTBEAT_GPIO_Port, 1},
    {LED_SETPOINT_Pin, LED_SETPOINT_GPIO_Port, 1},
    {LED_STATUS_Pin, LED_STATUS_GPIO_Port, 1},
    {LED_CTRL_MODE_Pin, LED_CTRL_MODE_GPIO_Port, 1},
    {LED_ERROR_Pin, LED_ERROR_GPIO_Port, 1},
    {LED_FATAL_Pin, LED_FATAL_GPIO_Port, 1},
};
static int32_t modes[] = {
    GPIO_MODE_INPUT,
    GPIO_MODE_ANALOG,
    GPIO_MODE_IT_RISING,
    GPIO_MODE_IT_FALLING,
    GPIO_MODE_IT_RISING_FALLING,
    GPIO_MODE_OUTPUT_PP,
    GPIO_MODE_OUTPUT_OD,
};

static int32_t pulls[] = {
    GPIO_NOPULL,
    GPIO_PULLUP,
    GPIO_PULLDOWN,
};

static int clocksInit = 0;

void Gpio_Init(GpioName_t pin, GpioMode_t mode, Gpio_Pull_t pull)
{
  if (!clocksInit)

  {
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    clocksInit = 1;
  }
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  HAL_GPIO_WritePin(pins[pin].pinPort, pins[pin].pin, pins[pin].initalState);

  GPIO_InitStruct.Pin = pins[pin].pin;
  GPIO_InitStruct.Mode = modes[mode];
  GPIO_InitStruct.Pull = pulls[pull];
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  HAL_GPIO_Init(pins[pin].pinPort, &GPIO_InitStruct);
}

void Gpio_PinReset(GpioName_t pin)
{
  if (pin < nrPins)
  {
    HAL_GPIO_WritePin(pins[pin].pinPort, pins[pin].pin, GPIO_PIN_RESET);
  }
}
void Gpio_PinSet(GpioName_t pin)
{
  if (pin < nrPins)
  {
    HAL_GPIO_WritePin(pins[pin].pinPort, pins[pin].pin, GPIO_PIN_SET);
  }
}
void Gpio_TogglePin(GpioName_t pin)
{
  if (pin < nrPins)
  {
    HAL_GPIO_TogglePin(pins[pin].pinPort, pins[pin].pin);
  }
}
