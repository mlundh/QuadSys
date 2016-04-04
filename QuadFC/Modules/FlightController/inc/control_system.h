/*
 * control_system.h
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
#ifndef MODULES_FLIGHTCONTROLLER_INC_CONTROL_SYSTEM_H_
#define MODULES_FLIGHTCONTROLLER_INC_CONTROL_SYSTEM_H_
#include "Utilities/inc/common_types.h"
#include "../inc/control_mode_handler.h"
typedef struct CtrlObj CtrlObj_t;

/**
 * Create a Control system object.
 * @return    Control system object containing everything needed by the controller.
 */
CtrlObj_t *Ctrl_Create(CtrlModeHandler_t* CtrlModeHandler);

/**
 * The constants (Kp, Ki and Kd) does not have a unit, but are expressed as 16.16 fixed point.
 * Output of the controller should lie in [0, 1<<16] where 1<<16 represent 100% control signal.
 *
 * @param obj Control object.
 * @return    0 if fail, 1 otherwise.
 */
uint8_t Ctrl_init(CtrlObj_t *obj);

/**
 * Execute the control system.
 * @param internals Current control object.
 * @param state     Current state data.
 * @param setpoint  Current setpoint.
 * @param u_signal  Output, control signal.
 */
void Ctrl_Execute(CtrlObj_t *internals, state_data_t *state, state_data_t *setpoint, control_signal_t *u_signal);

/**
 * Switch between the control schemes.
 *
 * @param param     Current control object.
 * @param newMode   Mode to change to.
 * @param state     Measurement.
 * @param u_signal  Control signal.
 */
void Ctrl_Switch(CtrlObj_t * param, CtrlMode_t newMode, state_data_t *state, control_signal_t *u_signal);

/**
 * Turn control on.
 * @param param Current control object.
 */
void Ctrl_On(CtrlObj_t * param);

/**
 * Turn control off.
 * @param param Current control object.
 */
void Ctrl_Off(CtrlObj_t * param);


//TODO move to motor control block and use number of motors.
/**
 * Allocate the control to different motors.
 * @param ctrl_signal     control signal to be allocated to motors.
 * @param motor_setpoint  Output, setpoint to each motor.
 */
void Ctrl_Allocate( control_signal_t *ctrl_signal, int32_t motor_setpoint[] );

#endif /* MODULES_FLIGHTCONTROLLER_INC_CONTROL_SYSTEM_H_ */
