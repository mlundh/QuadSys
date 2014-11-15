/*
 * state_handler.h
 *
 * Copyright (C) 2015 martin
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
  state_fault = 6,     // FC has encountered a serious problem and has halted.
  state_not_available = 7, // state information not available.
}state_t;

void State_InitStateHandler();
state_t State_GetCurrent();
BaseType_t State_Lock();
BaseType_t State_Unlock();
BaseType_t State_Fault();
BaseType_t State_Change(state_t state_req);


#endif /* STATE_HANDLER_H_ */
