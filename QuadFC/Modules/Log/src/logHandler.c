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

#include "Log/src/logHandler_private.h"
#include "Log/src/log_private.h"
#include "Modules/Utilities/inc/string_utils.h"


LogHandler_t* LogHandler_CreateObj(uint8_t num_children, eventHandler_t* evHandler, const char *obj_name, uint8_t isMaster)
{
  /* --- test code - remove when less heap is used for the logHandler --- */
  size_t heapSize = xPortGetFreeHeapSize();
  (void)heapSize;
  LogHandler_t *obj = pvPortMalloc( sizeof(LogHandler_t) );
  /* --- test code - End test code. --- */

  // only check obj, we are not required to have an event handler.
  if(!obj)
  {
    return NULL;
  }

  if(num_children > MAX_LOGGERS_PER_HANDLER)
  {
    return NULL;
  }
  obj->logQueue = xQueueCreate( LOG_QUEUE_LENGTH, LOG_QUEUE_ITEM_SIZE );
  obj->logNameQueueItems.logNames = pvPortMalloc(sizeof(logNames_t) * MAX_LOGGERS_PER_HANDLER); // TODO, fix memory consumption here.
  obj->logNameQueueItems.nrLogNames = 0;
  obj->logNameQueueItems.originator = obj;

  if(!obj->logQueue || !obj->logNameQueueItems.logNames)
  {
    return NULL;
  }

  if(isMaster)
  {
    obj->logNameQueue = xQueueCreate( LOG_NAME_QUEUE_LENGTH, LOG_NAME_QUEUE_ITEM_SIZE );
    obj->handlerArray = pvPortMalloc(sizeof(LogHandler_t*) * MAX_LOG_HANDLERS);
    obj->backend = LogBackend_CreateObj();
    obj->nrRegisteredHandlers = 1; // master is first.
    obj->logNameQueueItems.xMutex = NULL;
    if(!obj->backend || !obj->handlerArray )
    {
      return NULL;
    }
    obj->handlerArray[0] = obj;
  }
  else
  {
    obj->logNameQueue = NULL;
    obj->handlerArray = NULL;
    obj->backend = NULL; // Only the master has a backend.
    obj->nrRegisteredHandlers = 0;
    obj->logNameQueueItems.xMutex = xSemaphoreCreateMutex();

    if( !obj->logNameQueueItems.xMutex )
    {
      return NULL;
    }
  }


  param_obj_t * paramObj = Param_CreateObj(num_children, variable_type_NoType, readOnly, NULL, obj_name, Param_GetRoot(), NULL);
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


  obj->numChildrenAllocated = num_children;
  obj->registeredChildren = 0;
  obj->topId = 0;
  obj->lastTime = 0;
  obj->paramObject = paramObj;
  obj->evHandler = evHandler; // NULL if not using events
  if(obj->evHandler)
  {
    if(isMaster)
    {
      Event_RegisterCallback(obj->evHandler, eLog, LogHandler_HandleLogEvent, obj);
      obj->evHandler->subscriptions |= (1 << eLog); // create is called before initialize, therefore the subscription req function is not available.
    }
    else
    {
      Event_RegisterCallback(obj->evHandler, eLogNameReq, LogHandler_HandleNameReqEvent, obj);
      obj->evHandler->subscriptions |= (1 << eLogNameReq); // create is called before initialize, therefore the subscription req function is not available.

      Event_RegisterCallback(obj->evHandler, eLogStop, LogHandler_HandleStop, obj);
      obj->evHandler->subscriptions |= (1 << eLogStop); // create is called before initialize, therefore the subscription req function is not available.

    }
  }
  return obj;
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
    if(obj->backend)
    {
      // TODO read multiple entries at the same time ( all available) before sending to backend.
      logEntry_t queuedEntry = {0};
      while(xQueueReceive(obj->logQueue,&queuedEntry, 0)  == pdPASS)
      {
        if(!LogHandler_SendToBackend(obj, &queuedEntry))
        {
          result = 0;
        }
      }
    }
    else
    {
      obj->lastTime = entry.time;
      result = LogHandler_SendLogEvent(obj);
    }
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

uint8_t LogHandler_SendLogEvent(LogHandler_t*  obj)
{
  if(!obj)
  {
    return 0;
  }
  if(!obj->evHandler)
  {
    return 1; // OK to not have an event handler.
  }
  eventData_t event;
  event.eventNumber = eLog;
  event.data = obj->logQueue;
  return Event_Send(obj->evHandler, event);

}

uint8_t LogHandler_SendNameReqEvent(LogHandler_t*  obj)
{
  if(!obj)
  {
    return 0;
  }
  if(!obj->evHandler)
  {
    return 1; // OK to not have an event handler.
  }
  eventData_t event;
  event.eventNumber = eLogNameReq;
  event.data = obj->logNameQueue;
  return Event_Send(obj->evHandler, event);

  return 1;
}

uint8_t LogHandler_GetNameIdMapping(LogHandler_t*  obj, uint8_t* buffer, uint32_t buffer_length)
{
  if(!obj || !obj->backend)
  {
    return 0;//Only master is allowed to call this function.
  }
  uint8_t result = 1;

  result &= LogHandler_GetMapping(obj, obj->logNameQueueItems.logNames, MAX_LOGGERS_PER_HANDLER, &obj->logNameQueueItems.nrLogNames );

  result &= LogHandler_AppendNodeString(&obj->logNameQueueItems, buffer, buffer_length);

  result &= LogHandler_SendNameReqEvent(obj);

  if(!result)
  {
    return 0; // no need to go on if this failed.
  }

  if(!obj->evHandler)
  {
    return 1; // OK to not have an event handler.
  }
  result = 0; // reset result...
  logNameQueueItems_t nameItems = {};
  while(xQueueReceive(obj->logNameQueue, &nameItems, LOG_NAME_REQ_BLOCK_TIME)  == pdPASS)
  {
    if( xSemaphoreTake( nameItems.xMutex, 0 ) == pdTRUE )
    {
      for(uint32_t i = 0; i < nameItems.nrLogNames; i++)
      {
        LogHandler_UpdateId(obj, nameItems.originator, &(nameItems.logNames[i].id));
      }
      // handlerObj->logNameQueueItems is the shared resource, use and then give the mutex.
      result &= LogHandler_AppendNodeString(&nameItems, buffer, buffer_length);

      //TODO serialize and send to QuadGS.
      xSemaphoreGive( nameItems.xMutex );
    }
    else
    {
      result = 0;
    }
  }
  return result;
}


uint8_t LogHandler_HandleLogEvent(eventHandler_t* obj, void* data, eventData_t* event)
{
  if(!obj || ! data || !event)
  {
    return 0;
  }
  uint8_t result = 0;
  QueueHandle_t logQueue = event->data; // The data in a log event should always be a Queue handle.
  LogHandler_t* handlerObj = (LogHandler_t*)data; // The data should always be the handler object when a logEvent is received. The logHandler registers this properly iteself.
  result = LogHandler_ProcessDataInQueue(handlerObj, logQueue);

  return result;
}

uint8_t LogHandler_HandleNameReqEvent(eventHandler_t* obj, void* data, eventData_t* event)
{
  if(!obj || ! data || !event)
  {
    return 0;
  }

  uint8_t result = 1;
  QueueHandle_t logNameQueue = event->data; // The data in a log name event should always be a Queue handle.
  LogHandler_t* handlerObj = (LogHandler_t*)data; // The data should always be the handler object when a logEvent is received. The logHandler registers this properly iteself.

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

uint8_t LogHandler_HandleStop(eventHandler_t* obj, void* data, eventData_t* event)
{
  if(!obj || ! data || !event)
  {
    return 0;
  }
  uint8_t result = 1;
  LogHandler_t* handlerObj = (LogHandler_t*)data; // The data should always be the handler object when a logEvent is received. The logHandler registers this properly iteself.
  result = LogHandler_StopAllLogs(handlerObj);
  return result;
}
uint8_t LogHandler_ProcessDataInQueue(LogHandler_t* obj, QueueHandle_t logQueue)
{
  if(!obj->backend)
  {
    return 0;
  }
  logEntry_t queuedEntry = {0};

  while(xQueueReceive(logQueue, &queuedEntry, 0)  == pdPASS)
  {
    if(!LogHandler_SendToBackend(obj, &queuedEntry))
    {
      return 0;
    }
  }
  return 1;
}

param_obj_t* LogHandler_GetParameter(LogHandler_t* obj)
{
  return obj->paramObject;
}

uint8_t LogHandler_SendToBackend(LogHandler_t* obj, logEntry_t* log)
{
  if(!obj->backend)
  {
    return 0; // Only the master has a backend.
  }
  //Find the index of the handler
  if(!LogHandler_UpdateId(obj, log->handler, &log->id))
  {
    return 0;
  }

  return LogBackend_Report(obj->backend, log);
}

uint8_t LogHandler_UpdateId(LogHandler_t* obj, LogHandler_t* originator, uint32_t* id)
{
  if(!obj->backend)
  {
    return 0; // Only the master has a backend, and only master can update the id.
  }
  //Find the index of the handler
  int found = 0;
  int handlerIndex = 0;
  for(; handlerIndex < obj->nrRegisteredHandlers; handlerIndex++)
  {
    if(obj->handlerArray[handlerIndex] == originator)
    {
      found = 1;
      break;
    }
  }
  if(!found)
  {
    if(obj->nrRegisteredHandlers < MAX_LOG_HANDLERS)
    {
      obj->handlerArray[obj->nrRegisteredHandlers] = originator;
      obj->nrRegisteredHandlers++;
    }
    else
    {
      return 0;
    }
  }
  (*id) =  handlerIndex * MAX_LOGGERS_PER_HANDLER + (*id);
  return 1;
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

uint8_t LogHandler_AppendNodeString(logNameQueueItems_t *items, uint8_t *buffer, uint32_t buffer_length)
{
  if(!buffer || !items)
  {
    return 0;
  }
  uint32_t buf_index = (uint32_t)strlen((const char *)buffer);

  for(int i = 0; i < items->nrLogNames; i++)
  {
    //ensure that the name will fit in current buffer.
    if((buf_index + MAX_LOG_NODE_LENGTH + 1) > buffer_length)
    {
      return 0;
    }
    // append name and update buf_index.
    strncat( (char *) buffer , (const char *) items->logNames[i].name, (unsigned short) MAX_PARAM_NAME_LENGTH);

    //append value type <xx>
    strncat( (char *) buffer, (const char *) "<", (unsigned short) 1);
    uint8_t pVTTemp[MAX_VALUE_TYPE_LENGTH];
    snprintf((char *) pVTTemp, MAX_VALUE_TYPE_LENGTH, "%lu",(uint32_t)items->logNames[i].type);
    strncat( (char *) buffer, (const char *) pVTTemp, (unsigned short) MAX_LOG_TYPE_DIGITS);
    strncat( (char *) buffer, (const char *) ">", (unsigned short) 1);
    // append id "[xxx]"
    strncat( (char *) buffer, (const char *) "[", (unsigned short) 1);
    uint8_t pTemp[MAX_LOG_ID_DIGITS_ID];
    snprintf((char *) pTemp, MAX_LOG_ID_DIGITS_ID, "%lu",(uint32_t)items->logNames[i].id);
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_LOG_ID_DIGITS_ID);
    strncat( (char *) buffer, (const char *) "]", (unsigned short) 1);
    strncat( (char *) buffer, (const char *) "/", (unsigned short) 1);
  }

  return 1;
}

uint8_t LogHandler_AppendSerializedlogs(LogHandler_t* obj, uint8_t* buffer, uint32_t size)
{
  if(!obj || !buffer)
  {
    return 0;
  }
  if(size < MAX_LOG_ENTRY_LENGTH)
  {
    return 1;
  }
  uint8_t result = 1;
  uint32_t nrLogs = 0;

  {
    uint8_t nr_entries = size / MAX_LOG_ENTRY_LENGTH;
    logEntry_t entries[nr_entries];

    result &= LogHandler_Getlogs(obj, entries, nr_entries, &nrLogs);
    for(int i = 0; i < nrLogs; i++)
    {
      uint8_t pTemp[MAX_LOG_ENTRY_DIGITS_DATA];
      strncat( (char *) buffer, (const char *) "[", (unsigned short) 1);
      snprintf((char *) pTemp, MAX_LOG_ENTRY_DIGITS_ID, "%lu",(uint32_t)entries[i].id);
      strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_LOG_ENTRY_DIGITS_ID);
      strncat( (char *) buffer, (const char *) "]", (unsigned short) 1);

      strncat( (char *) buffer, (const char *) "[", (unsigned short) 1);
      snprintf((char *) pTemp, MAX_LOG_ENTRY_DIGITS_TIME, "%lu",(uint32_t)entries[i].time);
      strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_LOG_ENTRY_DIGITS_TIME);
      strncat( (char *) buffer, (const char *) "]", (unsigned short) 1);

      strncat( (char *) buffer, (const char *) "[", (unsigned short) 1);
      snprintf((char *) pTemp, MAX_LOG_ENTRY_DIGITS_DATA, "%lu",(uint32_t)entries[i].data);
      strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_LOG_ENTRY_DIGITS_DATA);
      strncat( (char *) buffer, (const char *) "]", (unsigned short) 1);
      strncat( (char *) buffer, (const char *) "/", (unsigned short) 1);
    }
  }
  // We might have used less than max length for the data fields, call the function
  // recursively until there is either no more space left in the buffer, or there is
  // no more data to get.
  if(result && (nrLogs > 0))
  {
    uint32_t length = strlen((const char*)buffer);
    if(length < size)
    {
      uint32_t newSize = size - length;
      result &= LogHandler_AppendSerializedlogs(obj, buffer, newSize);
    }

  }
  return result;
}
uint8_t LogHandler_Getlogs(LogHandler_t* obj, logEntry_t* logs, uint32_t size, uint32_t* nrLogs)
{
  if(!LogBackend_GetLog(obj->backend,logs,size,nrLogs))
  {
    return 0;
  }
  return 1;
}

uint8_t LogHandler_StopAllLogs(LogHandler_t* obj)
{
  if(!obj)
  {
    return 0; // only master is allowed to stop all!
  }

  uint8_t result = 1;
  for(int i = 0; (i < obj->registeredChildren); i++)
  {
    result &= Log_StopAllLogs(obj->children[i]);
  }
  if(obj->backend && obj->evHandler)
  {
    eventData_t event;
    event.eventNumber = eLogStop;
    event.data = obj->logQueue;
    result &= Event_Send(obj->evHandler, event);
  }
  return result;
}


