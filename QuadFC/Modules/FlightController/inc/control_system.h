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
#include "Messages/inc/common_types.h"
#include "Messages/inc/msg_enums.h"
#include "Parameters/inc/parameters.h"

typedef struct CtrlObj CtrlObj_t;


/**
 * The constants (Kp, Ki and Kd) does not have a unit, but are expressed as 16.16 fixed point.
 * Output of the controller should lie in [0, 1<<16] where 1<<16 represent 100% control signal.
 *
 * @param obj Control object.
 * @param evHandler event handler. Will only be used to initialize the internal mode handler.
 * @param param root parameter.
 * @return    0 if fail, 1 otherwise.
 */
CtrlObj_t* Ctrl_Create(param_obj_t* param);

/**
 * Execute the control system.
 * @param internals Current control object.
 * @param state     Current state data.
 * @param setpoint  Current setpoint.
 * @param u_signal  Output, control signal.
 * @param current_mode Current mode of the system.
 */
uint8_t Ctrl_Execute(CtrlObj_t *obj, state_data_t *state, state_data_t *setpoint
        , control_signal_t *u_signal, CtrlMode_t current_mode);
/**
 * Switch between the control schemes.
 *
 * @param param     Current control object.
 * @param newMode   Mode to change to.
 * @param state     Measurement.
 * @param u_signal  Control signal.
 */
uint8_t Ctrl_Switch(CtrlObj_t * param, CtrlMode_t newMode, state_data_t *state
        , control_signal_t *u_signal);
/**
 * Turn control on.
 * @param param Current control object.
 */
void Ctrl_On(CtrlObj_t * obj);

/**
 * Turn control off.
 * @param param Current control object.
 */
void Ctrl_Off(CtrlObj_t * obj);


/**
 * Allocate the control to different motors. 
 * Motor numbering: 
 * 
 * front
 * 4    1
 * 
 * 3    2
 *  back
 * 
 * motor rotation:
 * cw ccv
 * 
 * ccv cv
 * 
 * @param ctrl_signal     control signal to be allocated to motors.
 * @param motor_setpoint  Output, setpoint to each motor.
 * @param nrMotors        number of motors to allocate the control signal on.
 */
void Ctrl_Allocate( control_signal_t *ctrl_signal, uint32_t motor_setpoint[], uint32_t nrMotors);

#endif /* MODULES_FLIGHTCONTROLLER_INC_CONTROL_SYSTEM_H_ */
