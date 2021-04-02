/*
* AppLogTask.c
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

#include "../inc/AppLogTask.h"
#include "QuadFC/QuadFC_Serial.h"
#include "AppLog/inc/AppLogHandler.h"
#include "BoardConfig.h"

typedef struct AppLogTask
{
    eventHandler_t *evHandler;
    AppLogHandler_t *AppLogHandler;

} AppLogTask_t;

void AppLog_Task(void *pvParameters);

void AppLog_CreateTask(eventHandler_t *eventHandler)
{
    AppLogTask_t *taskParam = pvPortMalloc(sizeof(AppLogTask_t));
    if (!taskParam)
    {
        for (;;)
        {
        }
    }
    
    taskParam->evHandler = eventHandler;
    taskParam->AppLogHandler = AppLogHandler_Create(eventHandler);

    if (!taskParam->evHandler || !taskParam->AppLogHandler)
    {
        for (;;)
        {
        }
    }

    /* Create the freeRTOS tasks responsible for communication.*/
    portBASE_TYPE create_result;

    create_result = xTaskCreate(AppLog_Task,                      /* The task that implements the transmission of messages. */
                                (const char *const) "AppLog", /* Name, debugging only.*/
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

void AppLog_Task(void *pvParameters)
{
    AppLogTask_t * obj = (AppLogTask_t *) pvParameters;

    for (;;)
    {
        while(Event_Receive(obj->evHandler, 400))//TODO change to max delay...
        {}
    }
    
}