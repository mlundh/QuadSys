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
#define DEBUG


#include <string.h>
#include <stdio.h>
#include "../inc/ServiceTask.h"
#include "Parameters/inc/paramMasterHandler.h"
#include "Log/inc/logMaster.h"
#include "Debug/inc/debug_handler.h"
#include "Log/inc/logHandler.h"
#include "QuadFC/QuadFC_SPI.h"
#include "BoardConfig.h"
#include "QuadFC/QuadFC_Memory.h"
#include "Messages/inc/Msg_Log.h"
#include "AppLog/inc/AppLog.h"


void printBuffer(eventHandler_t *obj, uint8_t *packet)
{ // TODO!!
#ifdef DEBUG
    uint8_t array[400] = {0};
    uint32_t len = strlen((char*)packet);
    if(len > 400)
    {
        len = 400;
    }

    LOG_DBG_ENTRY( obj, "Log payload length: %ld", len);

    for (int i = 0; i < len; i++)
    {
        if ((packet[i] >= 33) && (packet[i] <= 127))
        {
            sprintf((char *)array + i, "%c", (char)packet[i]);
        }
        else
        {
            sprintf((char *)array + i, "%c", '0');
        }
    }
    LOG_DBG_ENTRY( obj, "Packet: %.*s", (int)220, array);
#endif
}
typedef struct serviceTask
{
    eventHandler_t *evHandler;
    LogMaster_t *logMaster;
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
    taskParam->paramMaster = ParamMaster_CreateObj(taskParam->evHandler);
    taskParam->debugHandler = Debug_CreateHandler(taskParam->evHandler);
    taskParam->logMaster = LogMaster_CreateObj(taskParam->evHandler, NULL);

    if (!taskParam->evHandler || !taskParam->logMaster || !taskParam->paramMaster)
    {
        for (;;)
        {
        }
    }


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