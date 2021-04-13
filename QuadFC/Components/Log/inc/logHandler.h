/*
 * logHandler.h
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
#ifndef MODULES_LOG_INC_LOGHANDLER_H_
#define MODULES_LOG_INC_LOGHANDLER_H_

#include <stdint.h>
#include "EventHandler/inc/event_handler.h"
#include "Parameters/inc/parameters.h"

#define MAX_LOGGERS_PER_HANDLER 20
#define LOG_NAME_REQ_BLOCK_TIME    (2UL / portTICK_PERIOD_MS )

typedef struct logHander LogHandler_t;

typedef struct log Log_t;
typedef struct logNames logNames_t;

typedef struct logNameQueueItems_
{
  logNames_t* logNames;
  LogHandler_t* originator;
  uint32_t    nrLogNames;
  SemaphoreHandle_t xMutex;
}logNameQueueItems_t;


typedef struct logEntry
{
    uint32_t time;          //! System time when the variable was sampled.
    uint32_t id;            //! id of the logged variable. This will, together with the logHandler id provide a system wide unique identifier.
    int32_t data;           //! data to stored. The log interface only supports int32_t logging, so all entries must be cast to that size.
    LogHandler_t* handler;  //! unique identifier of the log handler.
}logEntry_t;

#define LOG_QUEUE_LENGTH (32)
#define LOG_QUEUE_ITEM_SIZE ((sizeof(logEntry_t)))
#define LOG_NAME_QUEUE_LENGTH (16)
#define LOG_NAME_QUEUE_ITEM_SIZE ((sizeof(logNameQueueItems_t)))

/**
 * Create a log handler object. The object will either send log entries into the backend, or to the local
 * log entry queue depending on if it is a master or not. If the event is sent to the local log entry queue,
 * then an event is also sent, provided events are used. This message contains the queue from where the master
 * logHandler can retrieve the logs and send it to the backend.
 * @param num_children    Max number of children the handler can have.
 * @param evHandler       Event handler object.
 * @param param           Param object that is the root log object for the logHandler.
 * @param obj_name        Name of the handler.
 * @return
 */
LogHandler_t* LogHandler_CreateObj(uint8_t num_children, eventHandler_t* evHandler, param_obj_t* param, const char *obj_name);

/**
 * Free all memory created by the LogHandler_CreateObj function. No de-regestring is happening, so use with care.
 * @param obj
 */
void LogHandler_deleteHandler(LogHandler_t *obj);


/**
 * Set child of log handler.
 * @param current   Current log handler.
 * @param child     Child log object.
 * @return
 */
uint8_t LogHandler_setChild(LogHandler_t *current, Log_t *child);

/**
 * Report a log entry based on the log object given. The report function will write either to an internal queue and
 * send an event containing a pointer to the internal queue and the event number.
 * @param obj       LogHandler object.
 * @param log_obj   Log object to be used for creating a logEntry.
 * @return          1 if success, 0 otherwise.
 */
uint8_t LogHandler_report(LogHandler_t* obj, Log_t* log_obj);

/**
 * Function used to register a logger to the handler. Each id is unique only for the current handler, to get a
 * system unique id, combine with the LogHandler id.
 * @param obj
 * @return
 */
uint8_t LogHandler_RegisterLogger(LogHandler_t* obj, Log_t* logObj);

/**
 * Get the parameter of the logObject. This should only be used by the log interface.
 * @param obj
 * @return
 */
param_obj_t* LogHandler_GetParameter(LogHandler_t* obj);

#endif /* MODULES_LOG_INC_LOGHANDLER_H_ */
