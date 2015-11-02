/*
 * control_mode_handler.h
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
#ifndef MODULES_FLIGHTCONTROLLER_INC_CONTROL_MODE_HANDLER_H_
#define MODULES_FLIGHTCONTROLLER_INC_CONTROL_MODE_HANDLER_H_

#include "stdint.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

typedef struct CtrlModeHandler CtrlModeHandler_t;


typedef enum state{
  Control_mode_not_available = 0,
  Control_mode_rate = 1,
  Control_mode_attitude = 2
}CtrlMode_t;


/**
 * Create the mode handler object.
 * @return Handle to the created object.
 */
CtrlModeHandler_t* Ctrl_CreateModeHandler();

/**
 * Initialize the state handler. This will create queues used
 * in the implementation, and unlock the handler.
 * @param   obj Handle to the current mode handler object.
 */
void Ctrl_InitModeHandler(CtrlModeHandler_t* obj);

/**
 * Get the current state. Returns Ctrl_not_availible if it is not
 * possible to acces the state.
 * @param   obj Handle to the current mode handler object.
 * @return Current state. Ctrl_not_availible if fail.
 */
CtrlMode_t Ctrl_GetCurrentMode(CtrlModeHandler_t* obj);

/**
 * Set state to fault. Use this in case of a serious problem
 * requiring the flight controller to immediately stop the motors
 * and all control.
 *
 * If the vehicle is airborne at the time of call to Ctrl_Fault,
 * the vehicle will crash.
 * @param   obj Handle to the current mode handler object.
 * @return
 */
BaseType_t Ctrl_FaultMode(CtrlModeHandler_t* obj);

/**
 * @brief Request a state change.
 *
 * Request that the flight controller changes state. Will fail if
 * state transition is not allowed.
 *
 * @param   obj Handle to the current mode handler object.
 * @param Ctrl_req   New state beeing requested.
 * @return            pdTrue if ok, pdFalse otherwise.
 */
BaseType_t Ctrl_ChangeMode(CtrlModeHandler_t* obj, CtrlMode_t mode_req);


#endif /* MODULES_FLIGHTCONTROLLER_INC_CONTROL_MODE_HANDLER_H_ */
