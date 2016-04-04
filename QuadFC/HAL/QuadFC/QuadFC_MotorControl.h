/*
 * QuadFC_MotorControl.h
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
#ifndef HAL_INC_QUADFC_MOTORCONTROL_H_
#define HAL_INC_QUADFC_MOTORCONTROL_H_

#include "stdint.h"

/**
 * Motor control struct. Setpoint for control allocated control
 * should have a max value of 1<<16. Expressed in fixed point
 * this translates to 1<<30, (16+14).
 */

typedef struct MotorControl
{
  uint32_t nr_init_motors;
  uint8_t armed;
  int32_t *motorSetpoint;
  void *controlInternals;
} MotorControlObj;

/**
 * Initialize the motor controller.
 * @param obj       Pointer to the current motor control object.
 * @param nrMotors  Number of motors being used.
 * @return          1 of success, 0 otherwise.
 */
MotorControlObj * MotorCtrl_CreateAndInit(uint32_t nrMotors);

/**
 * Enable all motors. This arms the motors, waiting for a setpoint.
 * @param obj       Pointer to the current motor control object.
 * @return          1 of success, 0 otherwise.
 */
uint8_t MotorCtrl_Enable(MotorControlObj *obj);

/**
 * Disable all motors. The motors will not respond to setpoint.
 * @param obj       Pointer to the current motor control object.
 * @return          1 of success, 0 otherwise.
 */
uint8_t MotorCtrl_Disable(MotorControlObj *obj);

/**
 * Update setpoint of motors.  Motors will only run if a call to
 * MotorCtrl_Enable has been done first.
 * @param obj       Pointer to the current motor control object.
 * @param setpoint  Array of setpoints. Must be as long as nr_motors.
 * @return          1 of success, 0 otherwise.
 */
uint8_t MotorCtrl_UpdateSetpoint(MotorControlObj *obj);

#endif /* HAL_INC_QUADFC_MOTORCONTROL_H_ */
