/*
 * paramHandler.h
 *
 * Copyright (C) 2019 Martin Lundh
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


#ifndef COMPONENTS_PARAMETERS_SRC_PARAMHANDLER_H_
#define COMPONENTS_PARAMETERS_SRC_PARAMHANDLER_H_

typedef struct paramHander paramHander_t;

#include "Modules/EventHandler/inc/event_handler.h"
#include "Components/Parameters/inc/parameters_internal.h"


paramHander_t* ParamHandler_CreateObj(uint8_t num_children, eventHandler_t* evHandler,const char *obj_name, uint8_t master);

/**
 * Free all memory created by the ParamHander_CreateObj function.
 * No de-regestring is happening, so use with care.
 * @param obj
 */
void ParamHandler_DeleteHandler(paramHander_t *obj);

uint8_t ParamHandler_InitMaster(paramHander_t *obj);

uint8_t ParamHandler_Dump(paramHander_t *obj, uint8_t *buffer, uint32_t buffer_length, param_helper_t *helper);

param_obj_t* ParamHandler_GetParam(paramHander_t* obj);

uint8_t ParamHandler_SetFromRoot(paramHander_t *obj, uint8_t *buffer, uint32_t buffer_length);

uint8_t * ParamHandler_FindRoot(paramHander_t *obj, uint8_t *Buffer, uint32_t BufferLength);

#endif /* COMPONENTS_PARAMETERS_SRC_PARAMHANDLER_H_ */
