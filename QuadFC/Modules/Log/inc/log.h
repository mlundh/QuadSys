/*
 * log.h
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
#ifndef MODULES_LOG_INC_LOG_H_
#define MODULES_LOG_INC_LOG_H_

#include <stdint.h>
#include "Parameters/inc/parameters.h"
#include "Log/inc/logHandler.h"
typedef enum LogLevel
{
  Log_Off = 0,
  Log_On
}Log_level_t;

#define LOG_NAME_LENGTH 9


typedef struct log Log_t;

typedef struct logNames logNames_t;
/**
 * Create a log object. The object will be bound to the value, and you should
 * call Log_Report to create a log entry. The log object will register to either
 * a parent log object, or to a thread safe log handler, but never both. The
 * log object will register a parameter called logLevel that controls if the
 * logging is on or off.
 *
 * @param num_children  Max number of children the object can have.
 * @param type          Log value type.
 * @param value         Value to log. Note that it is not possible to bind a different value after creation.
 * @param parent        Parent log object. Should be NULL if a handler is given.
 * @param handler       Log handler. Should be NULL if a parent is given.
 * @param obj_name      Name of the log object.
 * @return              Pointer to the object if successfull, NULL otherwise.
 */
Log_t* Log_CreateObj(uint8_t num_children, Log_variable_type_t type, void* value, Log_t* parent, LogHandler_t* handler, const char *obj_name);

/**
 * Create a log report. Will create a log report based on the log object given. The log
 * object has a variable bound to it, and that variable will be used to create an entry.
 * @param obj           Log object to report from.
 * @return
 */
uint8_t Log_Report(Log_t* obj);


#endif /* MODULES_LOG_INC_LOG_H_ */
