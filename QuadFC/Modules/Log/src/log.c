/*
 * log.c
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

#include "Log/src/log_private.h"
#include <string.h>


Log_t* Log_CreateObj(uint8_t num_children, Log_variable_type_t type, void* value, Log_t* parent, LogHandler_t* handler, const char *obj_name)
{
  if((variable_type_NoType != type) && !value)
  {
    return NULL;
  }
  if((!parent && !handler) || (parent && handler))
  {
    return NULL;
  }
  Log_t *log_obj = pvPortMalloc( sizeof(Log_t) );
  if(!log_obj)
  {
    return NULL;
  }
  param_obj_t * parentParamObj = NULL;
  if(parent)
  {
    parentParamObj = parent->paramObject;
  }
  else
  {
    parentParamObj = LogHandler_GetParameter(handler);
  }
  param_obj_t * paramObj = Param_CreateObj(num_children, variable_type_uint8, readWrite, &log_obj->logLevel, obj_name, parentParamObj, NULL);
  if(!paramObj)
  {
    return NULL;
  }

  log_obj->parent = parent;
  log_obj->handler = handler;
  log_obj->children = NULL;
  log_obj->registeredChildren = 0;
  log_obj->value = value;
  log_obj->name = paramObj->group_name; // Same storage as for the param object!!
  log_obj->id = 0;
  log_obj->logLevel = 0;
  log_obj->type = type;
  log_obj->PeriodsSinceLastLog = 0;
  log_obj->paramObject = paramObj;
  if(num_children)
  {
    log_obj->children = pvPortMalloc(sizeof( Log_t *) * num_children);
    if(!log_obj->children)
    {
      return NULL;
    }
  }
  else
  {
    log_obj->children = NULL;
  }
  log_obj->num_children_allocated = num_children;



  if(parent)
  {
    if(!Log_SetChild(parent, log_obj))
    {
      return NULL;
    }
  }
  else
  {
    if(!LogHandler_setChild(handler, log_obj))
    {
      return NULL;
    }
  }

  // The handler argument might be NULL, make sure to get the correct handler.
  LogHandler_t* handler_ = Log_GetHandler(log_obj);
  if(!LogHandler_RegisterLogger(handler_, log_obj))
  {
    return NULL;
  }

  return log_obj;
}

uint8_t Log_SetChild(Log_t *current, Log_t *child)
{
  if(!current || !child || !(current->registeredChildren < current->num_children_allocated))
  {
    return 0; //TODO error
  }
  current->children[current->registeredChildren++] = child;
  return 1;
}

uint8_t Log_Report(Log_t* obj)
{
  if(!obj)
  {
    return 0;
  }
  // Do not log if there is no value assigned to the object, if the variable has no type or if the log level is 0.
  if(!obj->value ||
      obj->type == variable_type_NoType ||
      obj->logLevel <= 0)
  {
    return 1;
  }
  obj->PeriodsSinceLastLog++;

  if(obj->PeriodsSinceLastLog >= obj->logLevel)
  {
    obj->PeriodsSinceLastLog = 0;
  }
  else
  {
    return 1;
  }

  LogHandler_t* logH = Log_GetHandler(obj);
  if(!logH)
  {
    return 0;
  }
  return LogHandler_report(logH, obj);
}

LogHandler_t* Log_GetHandler(Log_t* obj)
{
  if(obj->handler)
  {
    return obj->handler;
  }
  else if(obj->parent)
  {
    return Log_GetHandler(obj->parent);
  }
  //ERROR!
  return NULL;
}

uint8_t Log_GetName(Log_t* obj, logNames_t* array, const uint32_t arrayLength, uint32_t* arrayIndex)
{
  if(!obj || !array || !arrayIndex || *arrayIndex+1 > arrayLength)
  {
    return 0;
  }
  uint8_t result = 1;
  // First get your own name and submit it into the array
  {
    logNames_t logName = {};
    logName.id = obj->id;
    logName.type = obj->type;
    strncpy((char*)logName.name, (char*)obj->name, MAX_PARAM_NAME_LENGTH);
    array[(*arrayIndex)++] = logName;
  }
  for(int i = 0; i < obj->registeredChildren; i++)
  {
    result &= Log_GetName(obj->children[i], array, arrayLength, arrayIndex);
  }
  return result;
}

uint8_t Log_StopAllLogs(Log_t* obj)
{
  uint8_t result = 1;
  obj->logLevel = 0;
  for(int i = 0; (i < obj->registeredChildren); i++)
  {
    result &= Log_StopAllLogs(obj->children[i]);
  }
  return result;
}

