/*
 * logHandler.c
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
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "Log/inc/log.h"
#include "Log/src/log_private.h"
#include "Utilities/inc/string_utils.h"
#include "Messages/inc/Msg_FcLog.h"
#include "Messages/inc/Msg_LogNameReq.h"


struct logHander
{
  QueueHandle_t         logQueue;
  logNameQueueItems_t   logNameQueueItems;
  eventHandler_t*       evHandler;
  param_obj_t*          param;
  uint32_t              topId;
  uint32_t              lastTime;
  param_obj_t *         paramObject;
  Log_t**               children;
  uint8_t               numChildrenAllocated;
  uint8_t               registeredChildren;
};


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
 * This handler will start all logs connected to the current handler.
 * @param obj     current event handler.
 * @param data    optional data. Here it is the current log handler.
 * @param event   event data. Not used by this handler.
 * @return
 */
uint8_t LogHandler_HandleStartMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);


/**
 * Send a log event. Only slave handlers should send log events, and only master handler
 * should listen to the event.
 * @param obj   current handler, must be slave.
 * @return      1 if success, 0 otherwise.
 */
uint8_t LogHandler_SendLogEvent(LogHandler_t*  obj);

/**
 * Get the id-name mapping. This function will add all mappings into the array of logNames_t, but not
 * serialize the objects for sending.
 * @param obj       Current object.
 * @param loggers   array of logName objects, this array will be filled by the function.
 * @param size      Size of the array.
 * @param arrIndex  Current index in the array.
 * @return
 */
uint8_t LogHandler_GetMapping(LogHandler_t* obj, logNames_t* logs, uint32_t size, uint32_t* arrIndex);


LogHandler_t* LogHandler_CreateObj(uint8_t num_children, eventHandler_t* evHandler, param_obj_t* param, const char *obj_name)
{

    LogHandler_t *obj = pvPortMalloc( sizeof(LogHandler_t) );

    // only check obj, we are not required to have an event handler.
    if(!obj || !evHandler)
    {
        return NULL;
    }

    if(num_children > MAX_LOGGERS_PER_HANDLER)
    {
        return NULL;
    }
    obj->logQueue = xQueueCreate( LOG_QUEUE_LENGTH, LOG_QUEUE_ITEM_SIZE );
    obj->logNameQueueItems.logNames = pvPortMalloc(sizeof(logNames_t) * MAX_LOGGERS_PER_HANDLER); 
    obj->logNameQueueItems.nrLogNames = 0;
    obj->logNameQueueItems.originator = obj;

    if(!obj->logQueue || !obj->logNameQueueItems.logNames)
    {
        return NULL;
    }

    obj->logNameQueueItems.xMutex = xSemaphoreCreateMutex();
    if( !obj->logNameQueueItems.xMutex )
    {
        return NULL;
    }
    
    param_obj_t * paramObj = Param_CreateObj(num_children, variable_type_NoType, readOnly, NULL, obj_name, param);
    if(!paramObj)
    {
        return NULL;
    }

    if(num_children > 0)
    {
        obj->children = pvPortMalloc(sizeof(Log_t*) * num_children);
        if(!obj->children)
        {
            return NULL;
        }
    }
    else
    {
        obj->children = NULL;
    }

    obj->numChildrenAllocated = num_children;
    obj->registeredChildren = 0;
    obj->topId = 0;
    obj->lastTime = 0;
    obj->paramObject = paramObj;
    obj->evHandler = evHandler;

    Event_RegisterCallback(obj->evHandler, Msg_LogNameReq_e, LogHandler_HandleNameReqMsg, obj);
    Event_RegisterCallback(obj->evHandler, LogStop_e, LogHandler_HandleStopMsg, obj);

    return obj;
}

void LogHandler_deleteHandler(LogHandler_t *obj)
{
    if(!obj)
    {
        return;
    }
    vQueueDelete(obj->logQueue);
    vPortFree(obj->logNameQueueItems.logNames);
    vSemaphoreDelete(obj->logNameQueueItems.xMutex);
    if(obj->children)
    {
        vPortFree(obj->children);
    }
    Param_DeleteObj(obj->paramObject);
    vPortFree(obj);
}

uint8_t LogHandler_setChild(LogHandler_t *current, Log_t *child)
{
    if(!current || !child || !(current->registeredChildren < current->numChildrenAllocated))
    {
        return 0;
    }
    current->children[current->registeredChildren++] = child;
    return 1;
}

uint8_t LogHandler_report(LogHandler_t* obj, Log_t* log_obj)
{
    // create a log entry and send it to the queue.
    if(!obj  || !log_obj)
    {
        return 0;
    }
    logEntry_t entry = {0};
    entry.data = *(uint32_t*)log_obj->value;
    entry.id = log_obj->id;
    entry.time = (uint32_t)xTaskGetTickCount();
    entry.handler = obj;

    if(xQueueSendToBack(obj->logQueue, &entry, 0) != pdPASS)
    {
        return 0;
    }
    uint8_t result = 1;
    // Send an event or write to the backend if needed.
    if(entry.time > obj->lastTime) // only send event if the RTOS tick has increased.
    {
        obj->lastTime = entry.time;
        result = LogHandler_SendLogEvent(obj);
    }
    return result;
}

uint8_t LogHandler_RegisterLogger(LogHandler_t* obj, Log_t* logObj)
{
    if(!obj || !logObj || (obj->topId > MAX_LOGGERS_PER_HANDLER))
    {
        return 0;
        // error...
    }
    logObj->id = obj->topId++;
    return 1;
}

param_obj_t* LogHandler_GetParameter(LogHandler_t* obj)
{
    return obj->paramObject;
}

uint8_t LogHandler_HandleNameReqMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }

    uint8_t result = 1;
    QueueHandle_t logNameQueue = Msg_LogNameReqGetData(msg);
    LogHandler_t* handlerObj = (LogHandler_t*)data; // The data should always be the handler object when a logEvent is received. The logHandler registers this properly iteself.

    if(!logNameQueue || !handlerObj)
    {
        return 0;
    }
    if( xSemaphoreTake( handlerObj->logNameQueueItems.xMutex, ( TickType_t ) 10 ) == pdTRUE )
    {
        // handlerObj->logNameQueueItems is the shared resource, update and then give the mutex.
        handlerObj->logNameQueueItems.nrLogNames = 0;
        result = LogHandler_GetMapping(handlerObj, handlerObj->logNameQueueItems.logNames, MAX_LOGGERS_PER_HANDLER, &handlerObj->logNameQueueItems.nrLogNames);

        xSemaphoreGive( handlerObj->logNameQueueItems.xMutex );

        // Now, send the resource to the other task.
        if(xQueueSendToBack(logNameQueue, &handlerObj->logNameQueueItems, 0) != pdPASS)
        {
            result = 0;
        }
    }
    else
    {
        result = 0;
    }
    return result;
}

uint8_t LogHandler_HandleStopMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    uint8_t result = 1;
    LogHandler_t* handlerObj = (LogHandler_t*)data; // The data should always be the handler object when a logEvent is received. The logHandler registers this properly iteself.
    for(int i = 0; (i < handlerObj->registeredChildren); i++)
    {
        result &= Log_StopAllLogs(handlerObj->children[i]);
    }
    return result;
}


uint8_t LogHandler_HandleStartMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    uint8_t result = 1;
    LogHandler_t* handlerObj = (LogHandler_t*)data; // The data should always be the handler object when a logEvent is received. The logHandler registers this properly iteself.
    for(int i = 0; (i < handlerObj->registeredChildren); i++)
    {
        result &= Log_StartAllLogs(handlerObj->children[i], 1);
    }
    return result;
}

uint8_t LogHandler_SendLogEvent(LogHandler_t*  obj)
{
    if(!obj)
    {
        return 0;
    }
    if(!obj->evHandler)
    {
        return 0; 
    }

    moduleMsg_t* event = Msg_FcLogCreate(FC_Broadcast_e, 0, obj->logQueue);
    return Event_Send(obj->evHandler, event);

}

uint8_t LogHandler_GetMapping(LogHandler_t* obj, logNames_t* loggers, uint32_t size, uint32_t* arrIndex)
{
    for(int i = 0; (i < obj->registeredChildren); i++)
    {
        Log_GetName(obj->children[i], loggers, size, arrIndex);
        if(*arrIndex > MAX_LOGGERS_PER_HANDLER)
        {
            return 0;
        }
    }
    return 1;
}