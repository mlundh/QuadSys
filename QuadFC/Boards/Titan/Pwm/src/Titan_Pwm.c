/*
* Titan_Pwm.c
*
* Copyright (C) 2021  Martin Lundh
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

#include "HAL/QuadFC/QuadFC_Pwm.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"

#define NR_PINS (8)
typedef struct pwm_internals
{
    uint8_t pinInitialized[NR_PINS];


    
} pwm_internals_t;

typedef struct
{
    TIM_TypeDef     timer;
    uint8_t         initialized;
    GPIO_TypeDef    GPIObank;
    uint32_t        pin;
} pwm_data_t;

static pwm_data_t pwmData[] = {
{TIM1, 0, GPIOA, LL_GPIO_PIN_9},
{TIM1, 0, GPIOE, LL_GPIO_PIN_9},
{TIM1, 0, GPIOE, LL_GPIO_PIN_13},
{TIM1, 0, GPIOE, LL_GPIO_PIN_14},
{TIM3, 0, GPIOB, LL_GPIO_PIN_0},
{TIM3, 0, GPIOB, LL_GPIO_PIN_1},
{TIM3, 0, GPIOB, LL_GPIO_PIN_4},
{TIM3, 0, GPIOB, LL_GPIO_PIN_5},

};

// PWM 1 - 4 is handled by timer 1. All these pins have to have the same configuration.
// PWM 5 - 8 id handled by timer 3. All these pins have to have the same configuration.


uint8_t PWM_Initialize(uint8_t pin, Pwm_Mode_t mode)
{


    




    LL_RCC_ClocksTypeDef clocks;
    LL_RCC_GetSystemClocksFreq(&clocks);

    switch (mode)
    {
    case pwm_50:
        /* code */
        break;
    case pwm_OneShot125:
        /* code */
        break;
    case pwm_OneShot45:
        /* code */
        break;    
    default:
        return 0;
    }
}


uint8_t PWM_InitializeCustom(uint8_t pin, uint8_t bitsResolution, uint32_t frequencyHz)
{

}


uint8_t PWM_SetDutyCycle(uint8_t pin, uint16_t duty)
{

}


uint8_t PWM_Enable(uint8_t pin)
{

}


uint8_t PWM_Disable(uint8_t pin)
{

}

