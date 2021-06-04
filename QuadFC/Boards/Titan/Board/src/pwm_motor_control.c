/*
 * PWM_motor_control
 *
 * Copyright (C) 2014  Martin Lundh
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

#include "QuadFC/QuadFC_MotorControl.h"
#include "Messages/inc/common_types.h"
#include "QuadFC/QuadFC_Pwm.h"
#include "BoardConfig.h"

#include "FreeRTOS.h"
#include <string.h>
#include "Parameters/inc/parameters.h"


struct MotorControl
{
  uint32_t scaleFactor;
};


/**
 * control signal resolution 100% = 1 << 16.
 *
 * throttle motor control, 100% = 2380.
 *
 * First step is to shift down from fixed point to integer:
 * thrust_from_state_v >> THRUST_SHIFT_FACTOR
 *
 * converting from control signal u thrust to motor setpoint y:
 *
 * y = x*2380/65536
 */

MotorControl_t * MotorCtrl_CreateAndInit(uint32_t nr_motors, param_obj_t * param)
{
    MotorControl_t* obj = pvPortMalloc(sizeof(MotorControl_t));
    if(!obj)
    {
        return NULL;
    }
    PWM_Initialize(PWM_PIN_1, pwm_OneShot125);
    PWM_Initialize(PWM_PIN_2, pwm_OneShot125);
    PWM_Initialize(PWM_PIN_3, pwm_OneShot125);
    PWM_Initialize(PWM_PIN_4, pwm_OneShot125);
    return obj;
}

uint8_t MotorCtrl_Enable(MotorControl_t *obj)
{
    if(!obj)
    {
        return 0;
    }
    PWM_Enable(PWM_PIN_1);
    return 1;
}

uint8_t MotorCtrl_Disable(MotorControl_t *obj)
{
    if(!obj)
    {
        return 0;
    }
    PWM_Disable(PWM_PIN_1);
    return 0;
}

uint8_t MotorCtrl_UpdateSetpoint(MotorControl_t *obj, uint32_t* motorSetpoints, uint32_t nrMotors)
{
    if(!obj || nrMotors < 4)
    {
        MotorCtrl_Disable(obj);
        return 0;
    }
    PWM_SetDutyCycle(PWM_PIN_1,motorSetpoints[0]);
    PWM_SetDutyCycle(PWM_PIN_2,motorSetpoints[1]);
    PWM_SetDutyCycle(PWM_PIN_3,motorSetpoints[2]);
    PWM_SetDutyCycle(PWM_PIN_4,motorSetpoints[3]);

    MotorCtrl_Enable(obj);
    return 1;
}

