/*
* paramMasterHandler.h
*
* Copyright (C) 2021  Martin Lundh
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

#ifndef __PARAMMASTERHANDLER_H__
#define __PARAMMASTERHANDLER_H__

typedef struct ParamMaster ParamMaster_t;

#include "EventHandler/inc/event_handler.h"

ParamMaster_t* ParamMaster_CreateObj(eventHandler_t* evHandler);

/**
 * Free all memory created by the ParamHander_CreateObj function.
 * No de-regestring is happening, so use with care.
 * @param obj
 */
void ParamMaster_DeleteHandler(ParamMaster_t *obj);

/**
 * The ParamMaster master must be initialized. It will then ask all other nodes if they have parameters.
 * @param obj
 * @return
 */
uint8_t ParamMaster_InitMaster(ParamMaster_t *obj);

#endif // __PARAMMASTERHANDLER_H__
