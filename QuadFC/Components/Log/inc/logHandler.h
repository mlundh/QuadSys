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

#define MAX_LOG_HANDLERS 10

#define MAX_LOG_NAME_LENGTH MAX_PARAM_NAME_LENGTH
#define MAX_LOG_NAME_EXTRA (7)
#define MAX_LOG_ID_DIGITS_ID (4)
#define MAX_LOG_TYPE_DIGITS (2)
#define MAX_LOG_NODE_LENGTH (MAX_LOG_NAME_LENGTH +  MAX_LOG_TYPE_DIGITS +MAX_LOG_NAME_EXTRA + MAX_LOG_ID_DIGITS_ID)

#define MAX_LOG_ENTRY_DIGITS_ID (4)
#define MAX_LOG_ENTRY_DIGITS_TIME (11)
#define MAX_LOG_ENTRY_DIGITS_DATA (11)
#define LOG_ENTRY_EXTRA (8)
#define MAX_LOG_ENTRY_LENGTH (MAX_LOG_ENTRY_DIGITS_ID + MAX_LOG_ENTRY_DIGITS_TIME + MAX_LOG_ENTRY_DIGITS_DATA + LOG_ENTRY_EXTRA)

#include <stdint.h>
#include "EventHandler/inc/event_handler.h"
#include "Log/inc/log_backend.h"
#include "Parameters/inc/parameters.h"

#define MAX_LOGGERS_PER_HANDLER 50
#define LOG_NAME_REQ_BLOCK_TIME    (2UL / portTICK_PERIOD_MS )
typedef struct logHander LogHandler_t;
typedef struct logNameQueueItems_ logNameQueueItems_t;
// put into a forward declaration file?
typedef struct log Log_t;
typedef struct logNames logNames_t;

typedef struct logEntry
{
    uint32_t time;          //! System time when the variable was sampled.
    uint32_t id;            //! id of the logged variable. This will, together with the logHandler id provide a system wide unique identifier.
    int32_t data;          //! data to stored. The log interface only supports int32_t logging, so all entries must be cast to that size.
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
 * @param isMaster        The master has a backend.
 * @return
 */
LogHandler_t* LogHandler_CreateObj(uint8_t num_children, eventHandler_t* evHandler, param_obj_t* param, const char *obj_name, uint8_t isMaster);

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
 * send an event (if events are used) containing a pointer to the internal queue and the event number, or if the
 * logHandler instance is a master, then write to the backend directly.
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
 * If a LogEvent is received then this function is used to process the log event properly. It will
 * first collect all the log entry from the queue specified in the event, and then write them to the backend.
 * @param obj   LogHandler object.
 * @param msg   Message.
 * @return
 */
uint8_t LogHandler_HandleLogMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

/**
 * This handler will serve all name requests. It will send all name-id mappings of the handler
 * to the given queue (the masters name queue).
 * @param obj     current handler.
 * @param data    optional data, here it contains the current log handler.
 * @param msg     message, the data field contains the response queue.
 * @return        1 success, 0 otherwise.
 */
uint8_t LogHandler_HandleNameReqMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

/**
 * This handler will stop all logs connected to the current handler.
 * @param obj     current event handler.
 * @param data    optional data. Here it is the current log handler.
 * @param event   event data. Not used by this handler.
 * @return
 */
uint8_t LogHandler_HandleStopMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

/**
 * Process data in the queue given. The queue should only contain logEntry entries. Use this
 * function to poll the different queues when an event system is not used. Only master is allowed
 * to use this function.
 * @param obj       Current logHandler object.
 * @param logQueue  Queue carrying logEntry data.
 * @return
 */
uint8_t LogHandler_ProcessDataInQueue(LogHandler_t* obj, QueueHandle_t logQueue);

/**
 * Get the parameter of the logObject. This should only be used by the log interface.
 * @param obj
 * @return
 */
param_obj_t* LogHandler_GetParameter(LogHandler_t* obj);

/**
 *
 * @param obj             Get the serialized name id mapping of all nodes associated with this handler.
 * @param buffer          Buffer to write into.
 * @param buffer_length   Length of that buffer.
 * @return                1 if success, 0 otherwise.
 */
uint8_t LogHandler_GetNameIdMapping(LogHandler_t*  obj, uint8_t* buffer, uint32_t buffer_length);

/**
 * Serialize the items in parameter "items" into the buffer. Make sure to have a buffer long enough to
 * contain all items. The function will null-terminate the string in buffer. The format of the string will
 * be similar to that of QuadFC parameters, but not contain any value type field, as all log IDs have
 * the same value type. Since the hierarchy does not matter here, the "child marker" "/" is only used to
 * separate different entries, and does not imply any particular relationship.
 *
 * example:
 *
 * A logger with name "PID_I_T" and id "230" would be serialized to:
 *
 * "PID_I_T[230]"
 *
 * and if we were to also serialize another logger called "U_SIG_X" with id "49" then the combined string
 * would look like:
 *
 * "PID_I_T[230]/U_SIG_X[49]"
 *
 * @param items         logNameQueueItems to serialize.
 * @param buffer        Buffer to write the serialization result to.
 * @param buffer_length Length of the buffer.
 * @return              1 indicates success, 0 faliure.
 */
uint8_t LogHandler_AppendNodeString(logNameQueueItems_t *items, uint8_t *buffer, uint32_t buffer_length);

/**
 * Get the serialized version of the log. Will fill the buffer with a null terminated string.
 * @param obj     LogHandler obj. Must be master.
 * @param buffer  Buffer to write the log entries into.
 * @param size    size of the buffer.
 * @return        1 if success, 0 otherwise.
 */
uint8_t LogHandler_AppendSerializedlogs(LogHandler_t* obj, uint8_t* buffer, uint32_t size);

/**
 * Get logs from the log backend.
 * @param obj         LogHandler obj. Must be the master.
 * @param logs        Buffer to write the logs into.
 * @param size        Size of the buffer.
 * @param nrLogs      Number of logs written.
 * @return            1 if success, 0 otherwise.
 */
uint8_t LogHandler_Getlogs(LogHandler_t* obj, logEntry_t* logs, uint32_t size, uint32_t* nrLogs);

/**
 * Stop all logging. All log levels are set to 0. Propagated throughout the system.
 * @param obj         master LogHandler object.
 * @return            1 is success, 0 otherwise.
 */
uint8_t LogHandler_StopAllLogs(LogHandler_t* obj);

/**
 * Start all local logs.
 * @param obj   LogHandler object.
 * @return      1 if sucssess, 0 otherwise.
 */
uint8_t LogHandler_StartAllLogs(LogHandler_t* obj);


#endif /* MODULES_LOG_INC_LOGHANDLER_H_ */
