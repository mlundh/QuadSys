/*
 * state_handler.h
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

#ifndef STATE_HANDLER_H_
#define STATE_HANDLER_H_

#include "stdint.h"
#include "Messages/inc/common_types.h"
#include "EventHandler/inc/event_handler.h"

#include "Messages/inc/Msg_FlightMode.h"
#include "Messages/inc/Msg_FlightModeReq.h"


typedef struct FlightModeHandler FlightModeHandler_t;

/**
 * Create the state handler object. Call this before the scheduler is
 * @return Handle to the created object.
 */
FlightModeHandler_t* FMode_CreateFmodeHandler(eventHandler_t* eHandler);

/**
 * Initialize the state handler. This will set the state to a fixed initial state.
 * @param evHandler   Event handler handle. Use if the call should send events, set to NULL otherwise.
 */
void FMode_InitFModeHandler(FlightModeHandler_t*  obj);

/**
 * Get the current state. Returns state_not_availible if it is not
 * possible to acces the state.
 *
 * @return Current state. State_not_availible if fail.
 */
FlightMode_t FMode_GetCurrent(FlightModeHandler_t* obj);

/**
 * Get the current flight mode. This should only be called by the owner thread.
 * @param obj           Current flight mode handler.
 * @param state_req     The required state.
 * @return              1 if sucess, 0 if not allowed.
 */
BaseType_t FMode_ChangeFMode(FlightModeHandler_t*  obj, FlightMode_t state_req);


/**
 * Set state to fault. Use this in case of a serious problem
 * requiring the flight controller to immediately stop the motors
 * and all control.
 *
 * If the vehicle is airborne at the time of call to State_Fault,
 * the vehicle will crash.
 * @param evHandler   Event handler handle. Use if the call should send events, set to NULL otherwise.
 * @return
 */
BaseType_t FMode_Fault(FlightModeHandler_t* obj);


#endif /* STATE_HANDLER_H_ */
