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
#include "Messages/inc/common_types.h"

#include "stm32f413xx.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"

#include "stm32f4xx_ll_bus.h"

typedef struct
{
    TIM_TypeDef *timer;
    uint32_t initialized;
    Pwm_Mode_t mode;
}timerInstance_t;

typedef struct
{
    timerInstance_t *timer;
    uint32_t pin;
    GPIO_TypeDef *gpioIf;
    uint32_t channelId;
    uint32_t pinAltFcn;
    uint32_t initialized;

} Titan_PwmChannel_t;

static timerInstance_t timers[] = {
    {TIM1, 0, 0},
    {TIM3, 0, 0}
};
#define FREQ_PWM_50 (500)
#define FREQ_ONE_SHOT_125 (4000)
#define FREQ_ONE_SHOT_42 (11900)

#define NR_ACTIVE_STEPS (2048)
#define START_STEP_ONE_SHOT (2048)

#define NR_TOTAL_STEPS_125 (4096)
#define NR_TOTAL_STEPS_42 (4096)
#define NR_TOTAL_STEPS_50 (4096)

#define MIN_PULSE_LENGTH_US_125 (125)
#define MAX_PULSE_LENGTH_US_125 (250) 

#define SHIFT_FACTOR_PWM (5) // 16 bit input, 11 bit output. 16-11 

// to reach max for both 42 and 125 we need 4000 steps, and active range is 2000 to 4000.

#define MIN_PULSE_LENGTH_US_42 (42)
#define MAX_PULSE_LENGTH_US_42 (84)






/**TIM1 GPIO Configuration
    PE9     ------> TIM1_CH1
    PE13     ------> TIM1_CH3
    PE14     ------> TIM1_CH4
    PA9     ------> TIM1_CH2
    */
/**TIM3 GPIO Configuration
    PB0     ------> TIM3_CH3
    PB1     ------> TIM3_CH4
    PB4     ------> TIM3_CH1
    PB5     ------> TIM3_CH2
    */

static Titan_PwmChannel_t Channels[] = {
    {&timers[0], LL_GPIO_PIN_9,  GPIOE, LL_TIM_CHANNEL_CH1, LL_GPIO_AF_1, 0},
    {&timers[0], LL_GPIO_PIN_9,  GPIOA, LL_TIM_CHANNEL_CH2, LL_GPIO_AF_1, 0},
    {&timers[0], LL_GPIO_PIN_13, GPIOE, LL_TIM_CHANNEL_CH3, LL_GPIO_AF_1, 0},
    {&timers[0], LL_GPIO_PIN_14, GPIOE, LL_TIM_CHANNEL_CH4, LL_GPIO_AF_1, 0},
    {&timers[1], LL_GPIO_PIN_4,  GPIOB, LL_TIM_CHANNEL_CH1, LL_GPIO_AF_2, 0},
    {&timers[1], LL_GPIO_PIN_5,  GPIOB, LL_TIM_CHANNEL_CH2, LL_GPIO_AF_2, 0},
    {&timers[1], LL_GPIO_PIN_0,  GPIOB, LL_TIM_CHANNEL_CH3, LL_GPIO_AF_2, 0},
    {&timers[1], LL_GPIO_PIN_1,  GPIOB, LL_TIM_CHANNEL_CH4, LL_GPIO_AF_2, 0},
};

// PWM 1 - 4 is handled by timer 1. All these pins have to have the same configuration.
// PWM 5 - 8 id handled by timer 3. All these pins have to have the same configuration.

void TitanPwm_InitTimer(timerInstance_t* timer, uint32_t PWM_Steps, uint32_t PWM_Frequency, Pwm_Mode_t mode);
void TitanPWM_InitPWMChannel(uint32_t channel);

uint8_t PWM_Initialize(uint8_t pin, Pwm_Mode_t mode)
{
    if(Channels[pin].timer->initialized)
    {
        TitanPWM_InitPWMChannel(pin);
        return 1;
    }
    Channels[pin].timer->mode=mode;

    switch (mode)
    {
    case pwm_50:
        TitanPwm_InitTimer(Channels[pin].timer, NR_TOTAL_STEPS_50, FREQ_PWM_50, pwm_50);
        TitanPWM_InitPWMChannel(pin);
        break;
    case pwm_OneShot125:
        TitanPwm_InitTimer(Channels[pin].timer, NR_TOTAL_STEPS_125, FREQ_ONE_SHOT_125, pwm_OneShot125);
        TitanPWM_InitPWMChannel(pin);
        break;
    case pwm_OneShot42:
        TitanPwm_InitTimer(Channels[pin].timer, NR_TOTAL_STEPS_42, FREQ_ONE_SHOT_42, pwm_OneShot42);
        TitanPWM_InitPWMChannel(pin);
        break;
    default:
        return 0;
    }
    return 1;
}
// Duty is 16 bit, one shot has 11 bit resolution. Drop the last bits to scale. 
// The active region of one shot is from 2048 to 4096, so we have to add 2048 
// as an offset. 
uint8_t PWM_SetDutyCycle(uint8_t pin, uint32_t duty)
{
    if(duty > MAX_U_SIGNAL )
    {
        duty = MAX_U_SIGNAL ;
    }
    switch (Channels[pin].timer->mode)
    {
    case pwm_50:
        break;
    case pwm_OneShot125:
        duty = START_STEP_ONE_SHOT + (duty>>SHIFT_FACTOR_PWM);
        break;
    case pwm_OneShot42:
        duty = START_STEP_ONE_SHOT + (duty>>SHIFT_FACTOR_PWM);
        break;
    default:
        return 0;
    }   
    if(duty > (NR_TOTAL_STEPS_125 - 2))
    {
        duty = NR_TOTAL_STEPS_125 - 2;
    }

    switch (Channels[pin].channelId)
    {
    case LL_TIM_CHANNEL_CH1:
        LL_TIM_OC_SetCompareCH1(Channels[pin].timer->timer, duty);
        break;
    case LL_TIM_CHANNEL_CH2:
        LL_TIM_OC_SetCompareCH2(Channels[pin].timer->timer, duty);
        break;
    case LL_TIM_CHANNEL_CH3:
        LL_TIM_OC_SetCompareCH3(Channels[pin].timer->timer, duty);
        break;
    case LL_TIM_CHANNEL_CH4:
        LL_TIM_OC_SetCompareCH4(Channels[pin].timer->timer, duty);
        break;
    default:
        break;
    }
    return 1;
}

uint8_t PWM_Enable(uint8_t pin)
{
    if(!Channels[pin].initialized)
    {
        return 0;
    }
    LL_TIM_EnableCounter(Channels[pin].timer->timer);
    return 1;
}

uint8_t PWM_Disable(uint8_t pin)
{
    if(!Channels[pin].initialized)
    {
        return 0;
    }
    LL_TIM_DisableCounter(Channels[pin].timer->timer);
    return 1;
}

void TitanPwm_InitTimer(timerInstance_t* timer, uint32_t PWM_Steps, uint32_t PWM_Frequency, Pwm_Mode_t mode)
{
    if(timer->initialized)
    {
        return;
    }

    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    uint32_t TIMER_Frequency = SystemCoreClock;
    uint32_t COUNTER_Frequency = PWM_Steps * PWM_Frequency;
    uint32_t PSC_Value = (TIMER_Frequency / COUNTER_Frequency) - 1;
    uint16_t ARR_Value = PWM_Steps - 1;

    /* Peripheral clock enable */
    if(timer->timer == TIM1)
    {
        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
    }
    else if (timer->timer == TIM3)
    {
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
    }

    if(timer->mode == pwm_OneShot42 || timer->mode == pwm_OneShot125)
    {
        LL_TIM_SetOnePulseMode(timer->timer, LL_TIM_ONEPULSEMODE_SINGLE);
        TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_DOWN;
    }
    else
    {
        TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    }

    TIM_InitStruct.Prescaler = PSC_Value;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_DOWN;
    TIM_InitStruct.Autoreload = ARR_Value;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    TIM_InitStruct.RepetitionCounter = 0;
    LL_TIM_Init(timer->timer, &TIM_InitStruct);
    LL_TIM_DisableARRPreload(timer->timer);
    LL_TIM_SetClockSource(timer->timer, LL_TIM_CLOCKSOURCE_INTERNAL);

    if(timer->timer == TIM1)
    {
        LL_TIM_BDTR_InitTypeDef TIM_BDTR_InitStruct;
        LL_TIM_BDTR_StructInit(&TIM_BDTR_InitStruct);
        TIM_BDTR_InitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_ENABLE;
        LL_TIM_BDTR_Init(TIM1, &TIM_BDTR_InitStruct);
    }
    timer->initialized = 1;
}

void TitamPwm_initGpio(uint32_t pin)
{
    if (Channels[pin].initialized)
    {
        return;
    }

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (Channels[pin].gpioIf == GPIOE)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
    }
    else if (Channels[pin].gpioIf == GPIOA)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    }
    else if (Channels[pin].gpioIf == GPIOB)
    {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    }

    GPIO_InitStruct.Pin = Channels[pin].pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = Channels[pin].pinAltFcn;
    LL_GPIO_Init(Channels[pin].gpioIf, &GPIO_InitStruct);

    
}

void TitanPWM_InitPWMChannel(uint32_t channel)
{
    if (Channels[channel].initialized)
    {
        return;
    }


    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.CompareValue = 0;
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;

    LL_TIM_OC_EnablePreload(Channels[channel].timer->timer, Channels[channel].channelId);
    LL_TIM_OC_Init(Channels[channel].timer->timer, Channels[channel].channelId, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(Channels[channel].timer->timer, Channels[channel].channelId);

    TitamPwm_initGpio(channel);
    LL_TIM_CC_EnableChannel(Channels[channel].timer->timer, Channels[channel].channelId);
    Channels[channel].initialized = 1;
}
