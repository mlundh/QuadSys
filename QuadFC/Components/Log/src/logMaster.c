/*
* logMaster.c
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

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "Log/inc/logMaster.h"
#include "Log/src/log_private.h"
#include "Utilities/inc/string_utils.h"
#include "Messages/inc/Msg_FcLog.h"
#include "Messages/inc/Msg_LogNameReq.h"
#include "Messages/inc/Msg_Log.h"

struct logMaster
{
  QueueHandle_t         logNameQueue;
  eventHandler_t*       evHandler;
  uint32_t              topId;
  LogBackEnd_t*         backend;
  struct logHander**    handlerArray;
  uint32_t              nrRegisteredHandlers;
  uint32_t              streamLogs;
  param_obj_t *         streamLogsParam;
};

LogMaster_t* LogMaster_CreateObj(eventHandler_t* evHandler, param_obj_t* param)
{

    LogMaster_t *obj = pvPortMalloc( sizeof(LogMaster_t) );

    if(!obj || !evHandler)
    {
        return NULL;
    }

    obj->evHandler = evHandler;
    obj->logNameQueue = xQueueCreate( LOG_NAME_QUEUE_LENGTH, LOG_NAME_QUEUE_ITEM_SIZE );
    obj->handlerArray = pvPortMalloc(sizeof(LogMaster_t*) * MAX_LOG_HANDLERS);
    obj->backend = LogBackend_CreateObj();
    obj->nrRegisteredHandlers = 0;
    obj->streamLogs = 1;
    if(!obj->backend || !obj->handlerArray )
    {
        return NULL;
    }
    Event_RegisterCallback(obj->evHandler, Msg_FcLog_e, LogMaster_HandleLogMsg, obj);
    Event_RegisterCallback(obj->evHandler, Msg_Log_e, LogMaster_HandleLog, obj);
    obj->streamLogsParam = Param_CreateObj(0, variable_type_uint32, readWrite, &obj->streamLogs, "StreamLogs", param);

    return obj;
}

void LogMaster_deleteHandler(LogMaster_t *obj)
{
    if(!obj)
    {
        return;
    }
    Param_DeleteObj(obj->streamLogsParam);
    vQueueDelete(obj->logNameQueue);
    vPortFree(obj->handlerArray);
    LogBackend_DeleteObj(obj->backend);
    vPortFree(obj);
}

#define NR_LOG_ENTRIES (10)
uint8_t LogMaster_HandleLogMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg )
    {
        return 0;
    }
    QueueHandle_t logQueue = Msg_FcLogGetData(msg);
    LogMaster_t* handlerObj = (LogMaster_t*)data; // The data should always be the handler object when a logEvent is received. The logMaster registers this properly iteself.
    
    if(!logQueue || !handlerObj)
    {
        return 0;
    }

    logEntry_t queuedEntry[NR_LOG_ENTRIES] = {0};
    uint32_t nrLogs = 0;
    uint8_t readMore = 1;
    while(readMore)
    {
        readMore = 0;
        while((xQueueReceive(logQueue, &queuedEntry[nrLogs], 0)  == pdPASS))
        {
            if(!LogMaster_UpdateId(handlerObj, queuedEntry[nrLogs].handler, &queuedEntry[nrLogs].id))
            {
                return 0;
            }

            LogBackend_Report(handlerObj->backend, &queuedEntry[nrLogs]);
            nrLogs++;
            if(nrLogs >= NR_LOG_ENTRIES)
            {
                readMore = 1;
                break;
            }
        }
        if(handlerObj->streamLogs)
        {
            moduleMsg_t *logMsg = Msg_LogCreate(GS_Log_e, 0, log_entry, NR_LOG_ENTRIES * MAX_LOG_ENTRY_LENGTH);
            Msg_SetRequireAck(logMsg, 0);
            uint8_t* payload = Msg_LogGetPayload(logMsg);
            payload[0] = '\0';
            uint32_t nr_entries = Msg_LogGetPayloadbufferlength(logMsg) / MAX_LOG_ENTRY_LENGTH;

            //sanity check
            if(nrLogs > nr_entries)
            {
                //error.
                break;
            }

            for(int j = 0; j < nrLogs; j++)
            {
                LogMaster_Serializelog(&queuedEntry[j], Msg_LogGetPayload(logMsg), Msg_LogGetPayloadbufferlength(logMsg));
            }
            uint16_t len = strlen((char *)Msg_LogGetPayload(logMsg));
            Msg_LogSetPayloadlength(logMsg, len);
            Event_Send(handlerObj->evHandler, logMsg);
        }
    }

    return 1;
}


uint8_t LogMaster_GetNameIdMapping(LogMaster_t*  obj, uint8_t* buffer, uint32_t buffer_length)
{
    if(!obj || !obj->backend)
    {
        return 0;
    }
    uint8_t result = 1;

    moduleMsg_t* event = Msg_LogNameReqCreate(FC_Broadcast_e, 0, obj->logNameQueue);
    result &= Event_Send(obj->evHandler, event);

    if(!result)
    {
        return 0; // no need to go on if this failed.
    }

    result = 0; // reset result...
    buffer[0] = '\0';
    logNameQueueItems_t nameItems = {0};
    while(xQueueReceive(obj->logNameQueue, &nameItems, LOG_NAME_REQ_BLOCK_TIME)  == pdPASS)
    {
        if( xSemaphoreTake( nameItems.xMutex, 0 ) == pdTRUE )
        {
            for(uint32_t i = 0; i < nameItems.nrLogNames; i++)
            {
                LogMaster_UpdateId(obj, nameItems.originator, &(nameItems.logNames[i].id));
            }
            // handlerObj->logNameQueueItems is the shared resource, use and then give the mutex.
            result &= LogMaster_AppendNodeString(&nameItems, buffer, buffer_length);

            xSemaphoreGive( nameItems.xMutex );
        }
        else
        {
            result = 0;
        }
    }
    return result;
}


uint8_t LogMaster_UpdateId(LogMaster_t* obj, LogHandler_t* originator, uint32_t* id)
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

uint8_t LogMaster_AppendNodeString(logNameQueueItems_t *items, uint8_t *buffer, uint32_t buffer_length)
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
        // append id "[xxx]"
        strcat( (char *) buffer, (const char *) "[");
        uint8_t pTemp[MAX_LOG_ID_DIGITS_ID];
        snprintf((char *) pTemp, MAX_LOG_ID_DIGITS_ID, "%lu",(uint32_t)items->logNames[i].id);
        strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_LOG_ID_DIGITS_ID);
        strcat( (char *) buffer, (const char *) "]");
        strcat( (char *) buffer, (const char *) "/");
    }

    return 1;
}
uint8_t LogMaster_Serializelog(logEntry_t* entry, uint8_t* buffer, uint32_t size)
{
    uint32_t length = strlen((const char*)buffer);
    uint32_t remainingSize = size - length;
    if(remainingSize < MAX_LOG_ENTRY_LENGTH)
    {
        return 1;
    }
    uint8_t pTemp[MAX_LOG_ENTRY_DIGITS_DATA];
    strcat( (char *) buffer, (const char *) "[");
    snprintf((char *) pTemp, MAX_LOG_ENTRY_DIGITS_ID, "%lu",(uint32_t)entry->id);
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_LOG_ENTRY_DIGITS_ID);
    strcat( (char *) buffer, (const char *) "]");
    strcat( (char *) buffer, (const char *) "[");
    snprintf((char *) pTemp, MAX_LOG_ENTRY_DIGITS_TIME, "%lu",(uint32_t)entry->time);
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_LOG_ENTRY_DIGITS_TIME);
    strcat( (char *) buffer, (const char *) "]");
    strcat( (char *) buffer, (const char *) "[");
    snprintf((char *) pTemp, MAX_LOG_ENTRY_DIGITS_DATA, "%lu",(uint32_t)entry->data);
    strncat( (char *) buffer, (const char *) pTemp, (unsigned short) MAX_LOG_ENTRY_DIGITS_DATA);
    strcat( (char *) buffer, (const char *) "]");
    strcat( (char *) buffer, (const char *) "/");
    return 1;
}

uint8_t LogMaster_AppendSerializedlogs(LogMaster_t* obj, uint8_t* buffer, uint32_t size)
{
    if(!obj || !buffer)
    {
        return 0;
    }
    uint32_t length = strlen((const char*)buffer);
    uint32_t remainingSize = size - length;
    if(remainingSize < MAX_LOG_ENTRY_LENGTH)
    {
        return 1;
    }
    uint8_t result = 1;
    uint32_t nrLogs = 0;

    {

        uint8_t nr_entries = remainingSize / MAX_LOG_ENTRY_LENGTH;
        logEntry_t entries[nr_entries];

        result &= LogMaster_Getlogs(obj, entries, nr_entries, &nrLogs);
        for(int i = 0; i < nrLogs; i++)
        {
            LogMaster_Serializelog(&entries[i], buffer, size);
        }
    }
    // We might have used less than max length for the data fields, call the function
    // recursively until there is either no more space left in the buffer, or there is
    // no more data to get.
    if(result && (nrLogs > 0))
    {
        uint32_t lengthAfter = strlen((const char*)buffer);
        if(lengthAfter < size)
        {
            result &= LogMaster_AppendSerializedlogs(obj, buffer, size);
        }

    }
    return result;
}

uint8_t LogMaster_Getlogs(LogMaster_t* obj, logEntry_t* logs, uint32_t size, uint32_t* nrLogs)
{
    if(!LogBackend_GetLog(obj->backend,logs,size,nrLogs))
    {
        return 0;
    }
    return 1;
}

uint8_t LogMaster_StopAllLogs(LogMaster_t* obj)
{
    if(!obj)
    {
        return 0; 
    }
    uint8_t result = 1;
    if(obj && obj->backend)
    {
        moduleMsg_t* event = Msg_Create(LogStop_e, FC_Broadcast_e);
        result &= Event_Send(obj->evHandler, event);
    }
    return result;
}


#define LOG_MSG_REPLY_LENGTH (255)
uint8_t LogMaster_HandleLog(eventHandler_t *obj, void *data, moduleMsg_t *msg)
{
    if (!obj || !data || !msg)
    {
        return 0;
    }

    LogMaster_t *logObj = (LogMaster_t *)data;
    uint8_t result = 0;

    switch (Msg_LogGetControl(msg))
    {
    case log_name:
    {
        result = 1;
        moduleMsg_t *reply = Msg_LogCreate(Msg_GetSource(msg), 0, log_name, LOG_MSG_REPLY_LENGTH);

        LogMaster_GetNameIdMapping(logObj, Msg_LogGetPayload(reply), Msg_LogGetPayloadbufferlength(reply));

        uint16_t len = strlen((char *)Msg_LogGetPayload(reply));
        Msg_LogSetPayloadlength(reply, len);

        Event_Send(obj, reply);
    }
    break;
    case log_entry:
    {
        result = 1;
        moduleMsg_t *reply = Msg_LogCreate(Msg_GetSource(msg), 0, log_entry, LOG_MSG_REPLY_LENGTH);
        uint8_t* payload = Msg_LogGetPayload(reply);
        payload[0]='\0';
        
        LogMaster_AppendSerializedlogs(logObj, payload, Msg_LogGetPayloadbufferlength(reply));
        uint16_t len = strlen((char *)Msg_LogGetPayload(reply));
        Msg_LogSetPayloadlength(reply, len);

        Event_Send(obj, reply);
    }
    break;
    case log_stopAll:
        LogMaster_StopAllLogs(logObj);
        result = 0;
        break;
    default:
        break;
    }
    return result;
}