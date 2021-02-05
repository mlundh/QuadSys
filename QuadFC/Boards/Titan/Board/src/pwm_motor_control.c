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

#include "FreeRTOS.h"
#include <string.h>
#include "Parameters/inc/parameters.h"

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


/** Required resolution = 2000 usable steps. */

/** PWM frequency in Hz */
#define PWM_FREQUENCY  (400)
/** PWM period value */
#define PERIOD_VALUE   (7100)
/** Initial duty cycle value */
#define INIT_DUTY_VALUE  (3100)
/** Initial dead time value */
#define INIT_DEAD_TIME   (0)
/** Maximum synchronous update period */
#define MAX_SYNC_UPDATE_PERIOD  (0)
/** Minimum input over INIT_DUTY_VALUE for the propellers to start rotating */
#define MIN_START_DUTY (200)
/**  Max allowed setponit */
#define MAX_SETPOINT (5680)

/**
 *  Contains identification information
 *  required for accessing a pwm channel.
 *
 */
typedef struct quad_pwm_parameters
{
    uint32_t pwm_gipo; /*< The pin of the channel */
    uint32_t pwm_flags; /*< The flags used to initialize the pin */
    uint32_t pwm_channel; /*< The channel of the pwm. */
} quad_pwm_parameters_t;

typedef struct pwm_internals
{
    quad_pwm_parameters_t *pwm_parameters;
//    pwm_channel_t pwm_channels;
    int32_t arming_duty_value;
    int32_t armed_duty_value;
    SemaphoreHandle_t xMutex;
} pwm_internals_t;

/**
 * Create a pwm motor control object.
 * @return
 */
pwm_internals_t *pwm_create();

/**
 * Convenience function to get a pointer to the internals struct.
 * @param obj   Current motor control object.
 * @return      Pointer to internals struct.
 */
pwm_internals_t *pwm_getInternals(MotorControlObj *obj);

/**
 * Convenience function to take the mutex assiciated with the current object.
 * @param obj   Current internals object.
 * @return      0 if failed to aquire mutex(do not use resource) 1 otherwise.
 */
uint8_t pwm_takeMutex(pwm_internals_t *obj);

/**
 * Convenience function to release the mutex assiciated with the current object.
 * @param obj   Current internals object.
 * @return      0 if failed, 1 otherwise.
 */
uint8_t pwm_giveMutex(pwm_internals_t *obj);


#define MAX_MOTORS (8)

pwm_internals_t *pwm_create()
{
    return NULL;
}





MotorControlObj * MotorCtrl_CreateAndInit(uint32_t nr_motors, param_obj_t * param)
{
    return NULL;
}

uint8_t MotorCtrl_Enable(MotorControlObj *obj)
{
    return 0;
}

uint8_t MotorCtrl_Disable(MotorControlObj *obj)
{
    return 0;
}

uint8_t MotorCtrl_UpdateSetpoint(MotorControlObj *obj)
{
    return 0;
}

pwm_internals_t *pwm_getInternals(MotorControlObj *obj)
{
    return NULL;
}


uint8_t pwm_takeMutex(pwm_internals_t *obj)
{
    return 0;
}


uint8_t pwm_giveMutex(pwm_internals_t *obj)
{
    return 0;
}


