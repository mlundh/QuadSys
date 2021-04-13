/*
* logMaster.h
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
#ifndef __LOGMASTER_H__
#define __LOGMASTER_H__

#include <stdint.h>
#include "EventHandler/inc/event_handler.h"
#include "Log/inc/log_backend.h"
#include "Parameters/inc/parameters.h"
#include "Log/inc/logHandler.h"

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

#define LOG_NAME_REQ_BLOCK_TIME    (2UL / portTICK_PERIOD_MS )

typedef struct logMaster LogMaster_t;

typedef struct logNames logNames_t;

#define LOG_QUEUE_LENGTH (32)
#define LOG_QUEUE_ITEM_SIZE ((sizeof(logEntry_t)))
#define LOG_NAME_QUEUE_LENGTH (16)
#define LOG_NAME_QUEUE_ITEM_SIZE ((sizeof(logNameQueueItems_t)))

/**
 * Create a log master object. The log master does not have any loggers associated with itself,
 * its job is to receive log entries from the log handlers and store them in the backend. It is 
 * also respocible for retreving data from the backend.
 * @param evHandler       Event handler object.
 * @return
 */
LogMaster_t* LogMaster_CreateObj(eventHandler_t* evHandler);

/**
 * Free all memory created by the LogMaster_CreateObj function. No de-regestring is happening, so use with care.
 * @param obj
 */
void LogMaster_deleteHandler(LogMaster_t *obj);

/**
 * If a LogEvent is received then this function is used to process the log event properly. It will
 * first collect all the log entry from the queue specified in the event, and then write them to the backend.
 * @param obj   LogMaster object.
 * @param msg   Message.
 * @return
 */
uint8_t LogMaster_HandleLogMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

/**
 *
 * @param obj             Get the serialized name id mapping of all nodes associated with this handler.
 * @param buffer          Buffer to write into.
 * @param buffer_length   Length of that buffer.
 * @return                1 if success, 0 otherwise.
 */
uint8_t LogMaster_GetNameIdMapping(LogMaster_t*  obj, uint8_t* buffer, uint32_t buffer_length);

/**
 * Update id to have system unique ids. All id:s sent by the slave handlers are only handler unique,
 * this function should be called by the master to update to system unique ids.
 * @param obj         current handler, must be master.
 * @param originator  originating handler.
 * @param id          id to update.
 * @return            1 if success, 0 otherwise.
 */
uint8_t LogMaster_UpdateId(LogMaster_t* obj, LogHandler_t* originator, uint32_t* id);

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
uint8_t LogMaster_AppendNodeString(logNameQueueItems_t *items, uint8_t *buffer, uint32_t buffer_length);

/**
 * Get the serialized version of the log. Will fill the buffer with a null terminated string.
 * @param obj     LogMaster obj. Must be master.
 * @param buffer  Buffer to write the log entries into.
 * @param size    size of the buffer.
 * @return        1 if success, 0 otherwise.
 */
uint8_t LogMaster_AppendSerializedlogs(LogMaster_t* obj, uint8_t* buffer, uint32_t size);

/**
 * Get logs from the log backend.
 * @param obj         LogMaster obj. Must be the master.
 * @param logs        Buffer to write the logs into.
 * @param size        Size of the buffer.
 * @param nrLogs      Number of logs written.
 * @return            1 if success, 0 otherwise.
 */
uint8_t LogMaster_Getlogs(LogMaster_t* obj, logEntry_t* logs, uint32_t size, uint32_t* nrLogs);

/**
 * Stop all logging. All log levels are set to 0. Propagated throughout the system.
 * @param obj         master LogMaster object.
 * @return            1 is success, 0 otherwise.
 */
uint8_t LogMaster_StopAllLogs(LogMaster_t* obj);

/**
 * Start all local logs.
 * @param obj   LogMaster object.
 * @return      1 if sucssess, 0 otherwise.
 */
uint8_t LogMaster_StartAllLogs(LogMaster_t* obj);



/**
 * Send a log event. Only slave handlers should send log events, and only master handler
 * should listen to the event.
 * @param obj   current handler, must be slave.
 * @return      1 if success, 0 otherwise.
 */
uint8_t LogMaster_SendLogEvent(LogMaster_t*  obj);

/**
 * Send a name request event. Should only be called from the master handler. All slave
 * handlers must respond to this event.
 * @param obj   current handler.
 * @return      1 if success, 0 otherwise.
 */
uint8_t LogMaster_SendNameReqEvent(LogMaster_t*  obj);

/**
 * Function only used by the master. The function will assign a unique id to each of
 * the log objects based on the objects id, and the assosiated handler.
 * @param obj   LogMaster object.
 * @param log   log entry to be written to the backend.
 * @return      1 if success, 0 otherwise.
 */
uint8_t LogMaster_SendToBackend(LogMaster_t* obj, logEntry_t* log);

uint8_t LogMaster_HandleLog(eventHandler_t *obj, void *data, moduleMsg_t *msg);


#endif // __LOGMASTER_H__
