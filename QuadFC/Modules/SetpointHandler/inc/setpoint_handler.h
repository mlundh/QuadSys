/*
 * setpoint_generator.h
 *
 * Copyright (C) 2016 Martin Lundh
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
#ifndef MODULES_SETPOINTGENERATOR_INC_SETPOINT_GENERATOR_H_
#define MODULES_SETPOINTGENERATOR_INC_SETPOINT_GENERATOR_H_

#include <stdint.h>
#include "Utilities/inc/common_types.h"
//#typedef struct SpObj SpObj_t;




/**
 * Create a setpointGenerator object.
 * @return    Control system object containing everything needed by the controller.
 */
SpObj_t *SpHandl_Create();

/**
 * Initialize the setpointGenerator object.
 * @param obj Control object.
 * @return    0 if fail, 1 otherwise.
 */
uint8_t SpHandl_init(SpObj_t *obj);

/**
 * Interface function for sending setpoints.If a still valid, higher setpoint
 * is already registered then the new setpoint will be ignored.
 * @param obj           Current setpoint object.
 * @param setpoint      Setpoint to send.
 * @param valid_for_ms  Time until when the setpoint should be considered too old.
 * @param prio          Priority of setpoint.
 * @return          1 if successful, 0 otherwise.
 */
uint8_t SpHandl_SetSetpoint(SpObj_t *obj, state_data_t* setpoint, uint8_t valid_for_ms, uint8_t prio );

/**
 * Interface function for getting new setpoints.
 * @param obj       Current setpoint object.
 * @param setpoint  Setpoint struct that will get populated.
 * @return          1 if successful, 0 otherwise.
 */
uint8_t SpHandl_GetSetpoint(SpObj_t *obj, state_data_t *setpoint, uint8_t timeout_ms );

#endif /* MODULES_SETPOINTGENERATOR_INC_SETPOINT_GENERATOR_H_ */
