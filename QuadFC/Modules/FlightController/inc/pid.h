/*
 * pid.h
 *
 * Copyright (C) 2015 Martin Lundh
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

/**
 * @file pid.h
 * Module implementing a PID control system.
 *
 * Input and output of the PID has 16 bit resolution. Internally
 * it uses fixed point arithmetics defined in my_math.h
 */
#ifndef MODULES_FLIGHTCONTROLLER_INC_PID_H_
#define MODULES_FLIGHTCONTROLLER_INC_PID_H_

#include "stdint.h"

typedef struct pidInternal pidInternal_t;

//TODO might want to move OutMin and OutMax and output to internal struct.
typedef struct pidConfig
{
  int32_t kp;
  int32_t ki;
  int32_t kd;
  int32_t OutMin;
  int32_t OutMax;
  uint32_t shift_factor;      //!<Shift factor for fixed point arithmetic.
  pidInternal_t *internal;

}pidConfig_t;


typedef enum PidMode
{
  pid_off = 0,
  pid_on = 1,
}Pid_mode_t;

/**
 * Create a PID object. Use this function to ensure correct memory
 * management and initialization.
 * @return pointer to new object.
 */
pidConfig_t* Pid_Create(int32_t kp, int32_t ki, int32_t kd, int32_t OutMin, int32_t OutMax);
/**
 * @brief Compute the output based on the input and parameters.
 *
 * Compute the result of the PID algorithm. The function uses
 * anti-windup, bumpless-transfer and derivative-of-measurement.
 *
 * It will also clamp the output to min/max limits if needed.
 *
 * Observe that it uses fixed point internally, and uses SHIFT_EXP in
 * internal calculations.
 *
 * @param pidConfig       pid controller object, includes output for now.
 * @param Measurement     Measurement from sensors. 16 bit resolution.
 * @param Setpoint        Desired setpoint. 16 bit resolution.
 */
void Pid_Compute(pidConfig_t *pidConfig, int32_t Measurement, int32_t Setpoint, int32_t *output);

/**
 * Set mode of PID controller. On and off are allowed. When turning on, the controller
 * needs to update its internal state to avoid a sudden increase in control action.
 *
 * @param pidConfig       pid controller object, includes output for now.
 * @param Mode            pid_on or pid_off allowed.
 * @param output          Current output from alternative control strategy.
 * @param Measurement     Measurement from sensors.
 */
void Pid_SetMode(pidConfig_t *pidConfig, Pid_mode_t Mode, int32_t output, int32_t Measurement);

#endif /* MODULES_FLIGHTCONTROLLER_INC_PID_H_ */
