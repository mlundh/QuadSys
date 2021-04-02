/*
* parameter_task.c
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
#include <string.h>
#include "../inc/ServiceTask.h"
#include "Parameters/inc/paramMasterHandler.h"
#include "Debug/inc/debug_handler.h"
#include "Log/inc/logHandler.h"
#include "QuadFC/QuadFC_SPI.h"
#include "BoardConfig.h"
#include "QuadFC/QuadFC_Memory.h"
#include "Messages/inc/Msg_Log.h"


typedef struct serviceTask
{
    eventHandler_t *evHandler;
    LogHandler_t *logHandler;
    ParamMaster_t *paramMaster;
    debug_handler_t* debugHandler;
} serviceTask_t;

void Service_Task(void *pvParameters);
uint8_t Service_HandleLog(eventHandler_t *obj, void *data, moduleMsg_t *msg);

void Service_CreateTask(eventHandler_t *eventHandler)
{
    serviceTask_t *taskParam = pvPortMalloc(sizeof(serviceTask_t));
    if (!taskParam)
    {
        for (;;)
        {
        }
    }
    
    taskParam->evHandler = eventHandler;
    taskParam->debugHandler = Debug_CreateHandler(taskParam->evHandler);
    taskParam->logHandler = LogHandler_CreateObj(0, taskParam->evHandler, NULL, "LogM", 1);
    taskParam->paramMaster = ParamMaster_CreateObj(taskParam->evHandler);

    if (!taskParam->evHandler || !taskParam->logHandler || !taskParam->paramMaster)
    {
        for (;;)
        {
        }
    }

    Event_RegisterCallback(taskParam->evHandler, Msg_Log_e, Service_HandleLog, taskParam->logHandler);

    uint8_t rspSpi = SpiMaster_Init(FRAM_MEM_SPI_BUS);
    if (!rspSpi)
    {
        /*Error - Could not create serial interface.*/

        for (;;)
        {
        }
    }
    Mem_Init();

    /* Create the freeRTOS tasks responsible for communication.*/
    portBASE_TYPE create_result;

    create_result = xTaskCreate(Service_Task,                      /* The task that implements the transmission of messages. */
                                (const char *const) "Service", /* Name, debugging only.*/
                                500,                             /* The size of the stack allocated to the task. */
                                (void *)taskParam,                 /* Pass the task parameters to the task. */
                                configMAX_PRIORITIES - 3,        /* The priority allocated to the task. */
                                NULL);                           /* No handle required. */

    if (create_result != pdTRUE)
    {
        /*Error - Could not create task.*/
        for (;;)
        {
        }
    }
}

void Service_Task(void *pvParameters)
{
    serviceTask_t * obj = (serviceTask_t *) pvParameters;

    ParamMaster_InitMaster(obj->paramMaster);
    for (;;)
    {
        while(Event_Receive(obj->evHandler, 400))//TODO change to max delay...
        {}
    }
    
}

#define LOG_MSG_REPLY_LENGTH (255)
uint8_t Service_HandleLog(eventHandler_t *obj, void *data, moduleMsg_t *msg)
{
    if (!obj || !data || !msg)
    {
        return 0;
    }

    LogHandler_t *logObj = (LogHandler_t *)data;
    uint8_t result = 0;

    switch (Msg_LogGetControl(msg))
    {
    case log_name:
    {
        result = 1;
        moduleMsg_t *reply = Msg_LogCreate(Msg_GetSource(msg), 0, log_name, LOG_MSG_REPLY_LENGTH);

        LogHandler_GetNameIdMapping(logObj, Msg_LogGetPayload(reply), Msg_LogGetPayloadbufferlength(reply));

        uint16_t len = strlen((char *)Msg_LogGetPayload(reply));
        Msg_LogSetPayloadlength(reply, len);

        Event_Send(obj, reply);
    }
    break;
    case log_entry:
    {
        result = 1;
        moduleMsg_t *reply = Msg_LogCreate(Msg_GetSource(msg), 0, log_entry, LOG_MSG_REPLY_LENGTH);

        LogHandler_AppendSerializedlogs(logObj, Msg_LogGetPayload(reply), Msg_LogGetPayloadbufferlength(reply));
        uint16_t len = strlen((char *)Msg_LogGetPayload(reply));
        Msg_LogSetPayloadlength(reply, len);

        Event_Send(obj, reply);
    }
    break;
    case log_stopAll:
        LogHandler_StopAllLogs(logObj);
        result = 0;
        break;
    default:
        break;
    }
    return result;
}