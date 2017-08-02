/*
 * logHandler_private.h
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
#ifndef MODULES_LOG_SRC_LOGHANDLER_PRIVATE_H_
#define MODULES_LOG_SRC_LOGHANDLER_PRIVATE_H_

#include "Log/inc/log.h"

typedef struct logNameQueueItems_
{
  logNames_t* logNames;
  LogHandler_t* originator;
  uint32_t    nrLogNames;
  SemaphoreHandle_t xMutex;
}logNameQueueItems_t;

struct logHander
{
  QueueHandle_t         logQueue;
  QueueHandle_t         logNameQueue;
  logNameQueueItems_t   logNameQueueItems;
  eventHandler_t*       evHandler;
  uint32_t              topId;
  uint32_t              lastTime;
  param_obj_t *         paramObject;
  LogBackEnd_t*         backend;
  struct logHander**    handlerArray;
  uint32_t              nrRegisteredHandlers;
  Log_t**               children;
  uint8_t               numChildrenAllocated;
  uint8_t               registeredChildren;
};

/**
 * Send a log event. Only slave handlers should send log events, and only master handler
 * should listen to the event.
 * @param obj   current handler, must be slave.
 * @return      1 if success, 0 otherwise.
 */
uint8_t LogHandler_SendLogEvent(LogHandler_t*  obj);

/**
 * Send a name request event. Should only be called from the master handler. All slave
 * handlers must respond to this event.
 * @param obj   current handler.
 * @return      1 if success, 0 otherwise.
 */
uint8_t LogHandler_SendNameReqEvent(LogHandler_t*  obj);

/**
 *
 * @param obj             Get the serialized name id mapping of all nodes assosiated with this handler.
 * @param buffer          Buffer to write into.
 * @param buffer_length   Length of that buffer.
 * @return                1 if success, 0 otherwise.
 */
uint8_t LogHandler_GetNameIdMapping(LogHandler_t*  obj, uint8_t* buffer, uint32_t buffer_length);

/**
 * Function only used by the master. The function will assign a unique id to each of
 * the log objects based on the objects id, and the assosiated handler.
 * @param obj   LogHandler object.
 * @param log   log entry to be written to the backend.
 * @return      1 if success, 0 otherwise.
 */
uint8_t LogHandler_SendToBackend(LogHandler_t* obj, logEntry_t* log);

/**
 * Update id to have system unique ids. All id:s sent by the slave handlers are only handler unique,
 * this function should be called by the master to update to system unique ids.
 * @param obj         current handler, must be master.
 * @param originator  originating handler.
 * @param id          id to update.
 * @return            1 if success, 0 otherwise.
 */
uint8_t LogHandler_UpdateId(LogHandler_t* obj, LogHandler_t* originator, uint32_t* id);

#endif /* MODULES_LOG_SRC_LOGHANDLER_PRIVATE_H_ */
