/*
 * logEventTester.c
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


/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "Test/Log/log_tester.h"
#include "Log/inc/logHandler.h"
#include "Log/src/log_private.h"
#include "Log/src/logHandler_private.h"
#include "Utilities/inc/string_utils.h"

#include "Test/Log/logEventTester.h"

#define LOG_EV_TEST_BLOCK_TIME    (50UL / portTICK_PERIOD_MS )
#define LOG_EV_TEST_STRING_LENGTH MAX_LOG_NODE_LENGTH * 16

/**
 * Struct only used here to pass parameters to the tasks.
 * The struct contains the tasks event handler, and a mutex
 * to which the task can respond to the test fw.
 */
typedef struct logEvTestTaskParams
{
  eventHandler_t* evHandler;
  LogHandler_t*   logHObj;
  SemaphoreHandle_t   xSemaphore;
  int32_t         logValue0;
  int32_t         logValue1;
}logEvTestTaskParams_t;

uint8_t LogEv_createTestTasks();

void LogEv_masterTask( void *pvParameters );
void LogEv_slaveTask( void *pvParameters );




void LogEv_GetTCs(TestFw_t* obj)
{
  TestFW_RegisterTest(obj, "Log C Handler", LogEv_TestMultipleThreads);
}

uint8_t LogEv_createTestTasks(TestFw_t* obj, SemaphoreHandle_t semaphore)
{
  logEvTestTaskParams_t * masterParam = pvPortMalloc(sizeof(logEvTestTaskParams_t));
  masterParam->xSemaphore = semaphore;

  masterParam->evHandler = Event_CreateHandler(NULL, 2);
  masterParam->logHObj = LogHandler_CreateObj(2, masterParam->evHandler,"logEvTM",1);
  masterParam->logValue0 = 0;
  masterParam->logValue1 = 0;

  uint8_t result = 1;
  if(!masterParam->logHObj || !masterParam->evHandler )
  {
    char tmpstr[50] = {0};
    snprintf (tmpstr, 50,"Could not create the master task params.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  portBASE_TYPE resultMasterTaskCreate = xTaskCreate( LogEv_masterTask,   /* The function that implements the task.  */
      (const char *const) "master",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
      500,                                           /* The stack size for the task*/
      masterParam,                                   /* pass params to task.*/
      configMAX_PRIORITIES-2,                        /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                        /* Handle to the task. Not used here and therefore NULL*/




  logEvTestTaskParams_t * slaveParam = pvPortMalloc(sizeof(logEvTestTaskParams_t));
  slaveParam->xSemaphore = NULL;
  slaveParam->evHandler = Event_CreateHandler(masterParam->evHandler->eventQueue, 0);
  slaveParam->logHObj = LogHandler_CreateObj(2,slaveParam->evHandler,"logEvTS",0);
  slaveParam->logValue0 = 0;
  slaveParam->logValue1 = 0;


  if(!slaveParam->logHObj || !slaveParam->evHandler )
  {
    char tmpstr[50] = {0};
    snprintf (tmpstr, 50,"Could not create the slave task params.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  portBASE_TYPE resultSlaveTaskCreate = xTaskCreate( LogEv_slaveTask,   /* The function that implements the task.  */
      (const char *const) "slave",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
      500,                                           /* The stack size for the task*/
      slaveParam,                                   /* pass params to task.*/
      configMAX_PRIORITIES-2,                        /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                        /* Handle to the task. Not used here and therefore NULL*/
  if((resultMasterTaskCreate != pdTRUE) && (resultSlaveTaskCreate != pdTRUE))
  {
    result = 0;
  }
  return result;
}


uint8_t LogEv_TestMultipleThreads(TestFw_t* obj)
{
  SemaphoreHandle_t   xSemaphore = xSemaphoreCreateBinary();

  if(!LogEv_createTestTasks(obj, xSemaphore))
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create the test tasks.\n");
    TestFW_Report(obj, tmpstr);
    return 0;
  }
  uint8_t result = 0;
  // Wait for the master task to signal test ok.
  if( xSemaphoreTake( xSemaphore, LOG_EV_TEST_BLOCK_TIME ) == pdTRUE )
  {
    result = 1;
  }
  else
  {
    char tmpstr[60] = {0};
    snprintf (tmpstr, 60,"Master task did not release the mutex in time.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }

  return result;
}

void LogEv_masterTask( void *pvParameters )
{
  logEvTestTaskParams_t * param = (logEvTestTaskParams_t*)(pvParameters);
  if(Event_InitHandler(param->evHandler) != 1)
  {
    for(;;)
    {
      // ERROR!
    }
  }
  if(!Event_SendAndWaitForAll(param->evHandler, eInitialize))
  {
    for(;;)
    {
    }
  }

  Log_t* logObj0 = Log_CreateObj(2,variable_type_int32, &param->logValue0,NULL,param->logHObj,"logM0");
  Log_t* logObj1 = Log_CreateObj(2,variable_type_int32, &param->logValue1,NULL,param->logHObj,"logM1");
  if(!logObj0 || !logObj1)
  {
    for(;;)
    {

    }
  }

  if(!Event_SendAndWaitForAll(param->evHandler, eInitializeDone))
  {
    for(;;)
    {
    }
  }


  // first, get name!
  uint8_t buffer[LOG_EV_TEST_STRING_LENGTH] = {0};
  buffer[0]='\0';
  LogHandler_GetNameIdMapping(param->logHObj, buffer, LOG_EV_TEST_STRING_LENGTH);


  uint8_t expected[LOG_EV_TEST_STRING_LENGTH] = {0};
  strcpy((char*)expected, "logM0[0]/logM1[1]/logS0[50]/logS1[51]/\0");
  uint8_t result = 0;
  if(strncmp((char*)buffer, (char*)expected, LOG_EV_TEST_STRING_LENGTH) == 0)
  {
    result = 1;
  }

  // then test logging.
  while(Event_Receive(param->evHandler, 0) == 1)
  {}


  logEntry_t entry[4] = {0};
  uint32_t nrLogs = 0;
  result &= LogHandler_Getlogs(param->logHObj,entry,4,&nrLogs);

  logEntry_t* entryP = entry;
  if(entryP->data != 11 ||
      entryP->id != 50 )
  {
    result = 0;
  }
  entryP++;
  if(entryP->data != 22 ||
      entryP->id != 51)
  {
    result = 0;
  }
  entryP++;
  if(entryP->data != 55 ||
      entryP->id != 50 )
  {
    result = 0;
  }
  entryP++;
  if(entryP->data != 66 ||
      entryP->id != 51)
  {
    result = 0;
  }
  if(result)
  {
    xSemaphoreGive( param->xSemaphore ); // Signal OK!
  }

  for(;;)
  {
    // Do nothing else..
    vTaskDelay(LOG_EV_TEST_BLOCK_TIME);
  }
}
void LogEv_slaveTask( void *pvParameters )
{

  logEvTestTaskParams_t * param = (logEvTestTaskParams_t*)(pvParameters);
  if(Event_InitHandler(param->evHandler) != 1)
  {
    for(;;)
    {
      // ERROR!
    }
  }
  if(!Event_SendAndWaitForAll(param->evHandler, eInitialize))
  {
    for(;;)
    {
    }
  }
  Log_t* logObj0 = Log_CreateObj(2,variable_type_int32, &param->logValue0,NULL,param->logHObj,"logS0");
  Log_t* logObj1 = Log_CreateObj(2,variable_type_int32, &param->logValue1,NULL,param->logHObj,"logS1");
  if(!logObj0 || !logObj1)
  {
    for(;;)
    {

    }
  }
  if(!Event_SendAndWaitForAll(param->evHandler, eInitializeDone))
  {
    for(;;)
    {
    }
  }


  // Enable the logs.
  uint8_t string[] = "/QuadFC/logEvTS/logS0[100]/../logS1[100]/0";
  Param_SetFromRoot(Param_GetRoot(),string ,strlen((const char*)string));

  //Log twice before reading and responding to events.
  param->logValue0 = 11;
  param->logValue1 = 22;
  Log_Report(logObj0);
  Log_Report(logObj1);
  param->logValue0 = 55;
  param->logValue1 = 66;
  Log_Report(logObj0);
  Log_Report(logObj1);


  Event_Receive(param->evHandler, portMAX_DELAY); // Now handle the name request event.

  for(;;)
  {
    vTaskDelay(LOG_EV_TEST_BLOCK_TIME);
  }
}
