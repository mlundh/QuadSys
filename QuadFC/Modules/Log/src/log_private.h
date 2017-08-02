/*
 * log_private.h
 *
 * Copyright (C) 2017 Martin Lundh
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
#ifndef MODULES_LOG_SRC_LOG_PRIVATE_H_
#define MODULES_LOG_SRC_LOG_PRIVATE_H_

#include "Log/inc/log.h"
struct log
{
  Log_t*  parent;
  LogHandler_t* handler;
  Log_t** children;
  uint8_t num_children_allocated;
  uint8_t registeredChildren;
  void* value;
  signed char *name;
  uint32_t id;
  uint8_t logLevel;
  Log_variable_type_t type;
  uint32_t period;
  param_obj_t * paramObject;
};


struct logNames
{
  signed char name[MAX_PARAM_NAME_LENGTH];
  uint32_t    id;
};

/**
 * Get the handler assosiated with the log object.
 * @param obj   Current log object.
 * @return      the handler, or NULL if fail.
 */
LogHandler_t* Log_GetHandler(Log_t* obj);

/**
 * Set child of the log object.
 * @param current
 * @param child
 * @return
 */
uint8_t Log_SetChild(Log_t *current, Log_t *child);

/**
 * Get the name-id mapping of this, and all child nodes.
 * @param obj
 * @param array
 * @param arrayLength
 * @param arrayIndex
 * @return
 */
uint8_t Log_GetName(Log_t* obj, logNames_t* array, const uint32_t arrayLength, uint32_t *arrayIndex);

#endif /* MODULES_LOG_SRC_LOG_PRIVATE_H_ */
