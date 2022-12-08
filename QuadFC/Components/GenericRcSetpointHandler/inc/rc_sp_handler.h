/*
* rc_sp_handler.h
*
* Copyright (C) 2022  Martin Lundh
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

#ifndef __RC_SP_HANDLER_H__
#define __RC_SP_HANDLER_H__


#include <stdint.h>
#include "Messages/inc/common_types.h"
#include "EventHandler/inc/event_handler.h"

typedef struct RcSpObj RcSpHandler_t;




/**
 * Create a setpointGenerator object.
 * @return    Control system object containing everything needed by the controller.
 */
RcSpHandler_t *RcSpHandl_Create(eventHandler_t* eHandler);

/**
 * Interface function for getting new setpoints. Only to be used by the owner of the module.
 * @param obj       Current setpoint object.
 * @param setpoint  Setpoint struct that will get populated.
 * @return          1 if successful, 0 otherwise.
 */
uint8_t RcSpHandl_GetSetpoint(RcSpHandler_t *obj, genericRC_t *setpoint, uint8_t timeout_ms );


#endif // __RC_SP_HANDLER_H__