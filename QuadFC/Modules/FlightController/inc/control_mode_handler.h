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
#include "Messages/inc/common_types.h"
#include "EventHandler/inc/event_handler.h"


typedef struct CtrlModeHandler CtrlModeHandler_t;

/**
 * Create the mode handler object.
 * @return Handle to the created object.
 */
CtrlModeHandler_t* Ctrl_CreateModeHandler(eventHandler_t* eHandler);

/**
 * Initialize the state handler. This will create queues used
 * in the implementation, and unlock the handler.
 * @param   obj Handle to the current mode handler object.
 * @param evHandler   Event handler handle. Use if the call should send events, set to NULL otherwise.
 *
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
 * @brief Request a state change.
 *
 * Request that the flight controller changes state. Will fail if
 * state transition is not allowed.
 *
 * @param   obj Handle to the current mode handler object.
 * @param evHandler   Event handler handle. Use if the call should send events, set to NULL otherwise.
 * @param Ctrl_req   New state beeing requested.
 * @return            1 if ok, 0 otherwise.
 */
uint8_t Ctrl_ChangeMode(CtrlModeHandler_t* obj, CtrlMode_t mode_req);


#endif /* MODULES_FLIGHTCONTROLLER_INC_CONTROL_MODE_HANDLER_H_ */
