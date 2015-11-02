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
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

typedef enum state{
  state_init = 1,      // FC is initializing.
  state_disarmed = 2,  // Disarmed state, no action except changing to another state.
  state_config = 3,    // Configuration state. FC can be configured.
  state_arming = 4,    // Motors are arming. No action other than change to error, disarmed or armed state.
  state_armed = 5,     // Armed state, FC is flight ready.
  state_disarming = 6, // Disarming.
  state_fault = 7,     // FC has encountered a serious problem and has halted.
  state_not_available = 0, // state information not available.
}state_t;

typedef struct StateHandler StateHandler_t;
/**
 * Create the state handler object.
 * @return Handle to the created object.
 */
StateHandler_t* State_CreateStateHandler();

/**
 * Initialize the state handler. This will create queues used
 * in the implementation, and unlock the handler.
 */
void State_InitStateHandler(StateHandler_t*);

/**
 * Get the current state. Returns state_not_availible if it is not
 * possible to acces the state.
 *
 * @return Current state. State_not_availible if fail.
 */
state_t State_GetCurrent(StateHandler_t*);

/**
 * Lock the state, do not allow change.
 * @return pdTrue if success.
 */
BaseType_t State_Lock(StateHandler_t*);

/**
 * Unlock state, allow a state transition.
 * @return pdTrue if success.
 */
BaseType_t State_Unlock(StateHandler_t*);

/**
 * Set state to fault. Use this in case of a serious problem
 * requiring the flight controller to immediately stop the motors
 * and all control.
 *
 * If the vehicle is airborne at the time of call to State_Fault,
 * the vehicle will crash.
 *
 * @return
 */
BaseType_t State_Fault(StateHandler_t*);

/**
 * @brief Request a state change.
 *
 * Request that the flight controller changes state. Will fail if
 * state transition is not allowed.
 *
 * @param state_req   New state beeing requested.
 * @return            pdTrue if ok, pdFalse otherwise.
 */
BaseType_t State_Change(StateHandler_t*, state_t state_req);


#endif /* STATE_HANDLER_H_ */
