/*
 * Log_tester.c
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
#include "Test/Log/log_tester.h"
#include "Log/src/log_private.h" // using private interface to be able to access the struct.
#include "Log/src/logHandler_private.h"
#include "Utilities/inc/string_utils.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

void Log_GetTCs(TestFw_t* obj)
{
  TestFW_RegisterTest(obj, "Log C Handler", LogHandler_TestCreate);
  TestFW_RegisterTest(obj, "Log C Obj", Log_TestCreate);
  TestFW_RegisterTest(obj, "Log Set Child", Log_TestSetChild);
  TestFW_RegisterTest(obj, "Log Report", Log_TestReport);
  TestFW_RegisterTest(obj, "Log Test Multiple Loggers", Log_TestMultipleLoggers);
  TestFW_RegisterTest(obj, "Log Test Multiple Handlers", Log_TestMultipleHandlers);
  TestFW_RegisterTest(obj, "Log Test Name", Log_TestGetName);
  TestFW_RegisterTest(obj, "Log Serialized", Log_TestGetNameSerialized);

}

uint8_t LogHandler_TestCreate(TestFw_t* obj)
{
  size_t heapSize = xPortGetFreeHeapSize();

  LogHandler_t* logHObjM = LogHandler_CreateObj(2,NULL,"TestRoot",1); // Not using eventHandler at the moment.
  uint8_t result = 1;
  if(!logHObjM)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create a logHandler object.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  if(!result ||
      !logHObjM->logQueue ||
      logHObjM->topId != 0 ||
      !logHObjM->paramObject ||
      !logHObjM->backend) // we are master, and should have a backend.
  {
    char tmpstr[50] = {0};
    snprintf (tmpstr, 50,"LogHandler object not correctly initiated.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  size_t heapSizeAfterM= xPortGetFreeHeapSize();
  size_t usedHeapM = heapSize - heapSizeAfterM;
  {

    char tmpstr[50] = {0};
    snprintf (tmpstr, 50,"LogHandler master uses %d bytes of heap.\n", usedHeapM);
    TestFW_Report(obj, tmpstr);
  }
  LogHandler_t* logHObjS = LogHandler_CreateObj(2,NULL,"TestRoot",0); // Not using eventHandler at the moment.
  if(!logHObjS)
  {
    char tmpstr[50] = {0};
    snprintf (tmpstr, 50,"Could not create a logHandler object.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  if(!result ||
      !logHObjS->logQueue ||
      logHObjS->topId != 0 ||
      !logHObjS->paramObject ||
      logHObjS->backend) // we are a slave, and should not have a backend.
  {
    char tmpstr[60] = {0};
    snprintf (tmpstr, 60,"LogHandler object not correctly initiated.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }

  size_t heapSizeAfterS = xPortGetFreeHeapSize();
  size_t usedHeapS = heapSizeAfterM - heapSizeAfterS;
  {
    char tmpstr[60] = {0};
    snprintf (tmpstr, 60,"LogHandler slave uses %d bytes of heap.\n", usedHeapS);
    TestFW_Report(obj, tmpstr);
  }
  return result;

}


uint8_t Log_TestCreate(TestFw_t* obj)
{
  LogHandler_t* logHObj = LogHandler_CreateObj(2,NULL,"TestH",1); // Not using event handler, but we are master.

  size_t heapSize = xPortGetFreeHeapSize();
  Log_t* logObj = Log_CreateObj(2,variable_type_NoType, NULL,NULL,logHObj,"TestO");
  size_t heapSizeAfterLog = xPortGetFreeHeapSize();
  size_t usedHeapLog = heapSize - heapSizeAfterLog;
  {
    char tmpstr[60] = {0};
    snprintf (tmpstr, 60,"A log obj uses %d bytes of heap.\n", usedHeapLog);
    TestFW_Report(obj, tmpstr);
  }
  uint8_t result = 1;
  if(!logObj)
  {
    char tmpstr[60] = {0};
    snprintf (tmpstr, 60,"Could not create a log object.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  if(!result ||
      logObj->parent != NULL ||
      logObj->handler != logHObj ||
      logObj->children == NULL ||
      logObj->num_children_allocated != 2 ||
      logObj->registeredChildren != 0 ||
      logObj->value != NULL ||
      strcmp((char*)logObj->name, "TestO") ||
      logObj->id != 0 ||
      logObj->logLevel != 0 ||
      logObj->type != variable_type_NoType ||
      logObj->PeriodsSinceLastLog != 0 ||
      logObj->paramObject == NULL )
  {
    result = 0;
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Log object not correctly initiated.\n");
    TestFW_Report(obj, tmpstr);
  }
  return result;
}

uint8_t Log_TestSetChild(TestFw_t* obj)
{
  uint8_t result = 1;

  LogHandler_t* logHObj = LogHandler_CreateObj(2,NULL,"TestH",1); // Not using event handler, but we are master.
  Log_t* logObj = Log_CreateObj(2,variable_type_NoType, NULL,NULL,logHObj,"root");
  Log_t* child = Log_CreateObj(0,variable_type_NoType, NULL,logObj,NULL,"root");

  if(!logObj || !child)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create a log object.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }

  if(logObj->children[0] != child)
  {
    result = 0;
  }
  return result;
}


uint8_t Log_TestReport(TestFw_t* obj)
{
  uint8_t result = 1;
  LogHandler_t* logHObj = LogHandler_CreateObj(2,NULL,"Report",1); // Not using event handler, but we are master.
  int32_t data = 9;
  Log_t* logObj = Log_CreateObj(2,variable_type_int32, &data,NULL,logHObj,"root1");
  if(!logObj)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create a log object.\n");
    TestFW_Report(obj, tmpstr);
    return 0;
  }
  // set the log level
  uint8_t string[] = "/QuadFC/Report/root1[100]/\0";
  Param_SetFromRoot(Param_GetRoot(),string ,strlen((const char*)string));

  // enter critical to ensure same tick...
  taskENTER_CRITICAL();
  uint32_t time = (uint32_t)xTaskGetTickCount();
  uint8_t report_res = Log_Report(logObj);
  taskEXIT_CRITICAL();

  if(!report_res)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create a log entry..\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  if(result && report_res)
  {
    logEntry_t entry[5] = {0};
    uint32_t nrLogs = 0;
    if(!LogBackend_GetLog(logHObj->backend,entry,5,&nrLogs))
    {
      return 0;
    }
    logEntry_t* entryP = entry;
    if(entryP->data != 9 ||
        entryP->id != 0 ||
        entryP->time != time )
    {
      char tmpstr[40] = {0};
      snprintf (tmpstr, 40,"First log entry not correct.\n");
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
  }


  // enter critical to ensure same tick...
  taskENTER_CRITICAL();
  time = (uint32_t)xTaskGetTickCount();
  data = 879;
  report_res = Log_Report(logObj);
  taskEXIT_CRITICAL();

  if(!report_res)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create a log entry.\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  if(result && report_res)
  {
    logEntry_t entry[5] = {0};
    uint32_t nrLogs = 0;
    if(!LogBackend_GetLog(logHObj->backend,entry,5,&nrLogs))
    {
      return 0;
    }
    logEntry_t* entryP = entry;
    if(entryP->data != 879 ||
        entryP->id != 0 ||
        entryP->time != time )
    {
      char tmpstr[40] = {0};
      snprintf (tmpstr, 40,"Second log entry not correct.\n");
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
  }
  return result;
}


uint8_t Log_TestMultipleLoggers(TestFw_t* obj)
{
  uint8_t result = 1;
  LogHandler_t* logHObj = LogHandler_CreateObj(2,NULL,"Report1",1); // Not using event handler, but we are master.
  int32_t data = 9;
  Log_t* logObj = Log_CreateObj(2,variable_type_int32, &data,NULL,logHObj,"root1");
  int32_t data2 = 800;
  Log_t* logObj2 = Log_CreateObj(2,variable_type_int32, &data2,logObj,NULL,"child1");
  if(!logObj || !logObj2)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create log objects.\n");
    TestFW_Report(obj, tmpstr);
    return 0;
  }
  // set the log level
  uint8_t string[] = "/QuadFC/Report1/root1[100]/\0";
  Param_SetFromRoot(Param_GetRoot(),string ,strlen((const char*)string));
  uint8_t string2[] = "/QuadFC/Report1/root1/child1[100]/\0";
  Param_SetFromRoot(Param_GetRoot(),string2 ,strlen((const char*)string2));
  // enter critical to ensure same tick...
  taskENTER_CRITICAL();
  uint32_t time = (uint32_t)xTaskGetTickCount();
  uint8_t report_res = Log_Report(logObj);
  report_res &= Log_Report(logObj2);
  taskEXIT_CRITICAL();

  if(!report_res)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create a log entry..\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  if(result && report_res)
  {
    logEntry_t entry[5] = {0};
    uint32_t nrLogs = 0;
    if(!LogBackend_GetLog(logHObj->backend,entry,5,&nrLogs))
    {
      return 0;
    }
    logEntry_t* entryP = entry;
    if(entryP->data != data ||
        entryP->id != 0 ||
        entryP->time != time )
    {
      char tmpstr[40] = {0};
      snprintf (tmpstr, 40,"First log entry not correct.\n");
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
    entryP++;
    if(entryP->data != data2 ||
        entryP->id != 1 ||
        entryP->time != time )
    {
      char tmpstr[40] = {0};
      snprintf (tmpstr, 40,"Second log entry not correct.\n");
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
  }
  return result;
}
uint8_t Log_TestMultipleHandlers(TestFw_t* obj)
{
  uint8_t result = 1;
  LogHandler_t* logHObjT1 = LogHandler_CreateObj(2,NULL,"TH1",1); // Not using event handler, we are master.
  int32_t data = 9;
  Log_t* logObjT1 = Log_CreateObj(2,variable_type_int32, &data,NULL,logHObjT1,"LogT1");

  LogHandler_t* logHObjT2 = LogHandler_CreateObj(2,NULL,"TH2",0); // Not using event handler, we are not master.
  int32_t data2 = 800;
  Log_t* logObjT2 = Log_CreateObj(2,variable_type_int32, &data2,NULL,logHObjT2,"LogT2");
  if(!logObjT1 || !logObjT2 || !logHObjT1 ||! logHObjT2)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create log objects.\n");
    TestFW_Report(obj, tmpstr);
    return 0;
  }
  // set the log level
  uint8_t string[] = "/QuadFC/TH1/LogT1[100]/\0";
  Param_SetFromRoot(Param_GetRoot(),string ,strlen((const char*)string));
  uint8_t string2[] = "/QuadFC/TH2/LogT2[100]/\0";
  Param_SetFromRoot(Param_GetRoot(),string2 ,strlen((const char*)string2));
  // enter critical to ensure same tick...
  taskENTER_CRITICAL();
  uint32_t time = (uint32_t)xTaskGetTickCount();
  uint8_t report_res = Log_Report(logObjT1);
  report_res &= Log_Report(logObjT1);
  report_res &= Log_Report(logObjT2);
  report_res &= Log_Report(logObjT2);
  taskEXIT_CRITICAL();

  // Since we do not use event handlers we have to manually poll the log queues.
  // Remember, logHObj1 is the master.
  LogHandler_ProcessDataInQueue(logHObjT1, logHObjT2->logQueue);
  // force immediate processing of own queue too.
  LogHandler_ProcessDataInQueue(logHObjT1, logHObjT1->logQueue);

  if(!report_res)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create a log entry..\n");
    TestFW_Report(obj, tmpstr);
    result = 0;
  }
  if(result && report_res)
  {
    logEntry_t entry[5] = {0};
    uint32_t nrLogs = 0;
    if(!LogBackend_GetLog(logHObjT1->backend,entry,5,&nrLogs))
    {
      return 0;
    }
    logEntry_t* entryP = entry;
    if(entryP->data != data ||
        entryP->id != 0 ||
        entryP->time != time )
    {
      char tmpstr[40] = {0};
      snprintf (tmpstr, 40,"First log entry not correct.\n");
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
    entryP++;
    if(entryP->data != data ||
        entryP->id != 0 ||
        entryP->time != time )
    {
      char tmpstr[40] = {0};
      snprintf (tmpstr, 40,"Second log entry not correct.\n");
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
    entryP++;
    if(entryP->data != data2 ||
        entryP->id != MAX_LOGGERS_PER_HANDLER ||
        entryP->time != time )
    {
      char tmpstr[40] = {0};
      snprintf (tmpstr, 40,"Third log entry not correct.\n");
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
    entryP++;
    if(entryP->data != data2 ||
        entryP->id != MAX_LOGGERS_PER_HANDLER ||
        entryP->time != time )
    {
      char tmpstr[40] = {0};
      snprintf (tmpstr, 40,"fourth log entry not correct.\n");
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
  }

  return result;
}



uint8_t Log_TestGetName(TestFw_t* obj)
{
  uint8_t result = 1;
  LogHandler_t* logHObj = LogHandler_CreateObj(2,NULL,"GetNameH",1); // Not using event handler, but we are master.
  int32_t data = 9;
  //Create a bunch of loggers...
  Log_t* logObj10 = Log_CreateObj(2,variable_type_int32, &data,NULL,logHObj,"1.0");
  Log_t* logObj11 = Log_CreateObj(2,variable_type_int32, &data,logObj10,NULL,"1.1");
  Log_t* logObj12 = Log_CreateObj(2,variable_type_int32, &data,logObj10,NULL,"1.2");


  Log_t* logObj20 = Log_CreateObj(2,variable_type_int32, &data,NULL,logHObj,"2.0");
  Log_t* logObj21 = Log_CreateObj(2,variable_type_int32, &data,logObj20,NULL,"2.1");
  Log_t* logObj22 = Log_CreateObj(2,variable_type_int32, &data,logObj20,NULL,"2.2");
  Log_t* logObj220 = Log_CreateObj(2,variable_type_int32, &data,logObj22,NULL,"2.2.0");


  if(!logObj10 || !logObj11 || !logObj12 || !logObj20 || !logObj21 || !logObj22 || !logObj220)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create log objects.\n");
    TestFW_Report(obj, tmpstr);
    return 0;
  }

  logNames_t loggers[MAX_LOGGERS_PER_HANDLER];
  uint32_t arrIndex = 0;
  LogHandler_GetMapping(logHObj, loggers, MAX_LOGGERS_PER_HANDLER, &arrIndex);

  logNames_t loggersVerify[8];

  logNames_t tmpName = {};

  // First logger...
  tmpName.id = 0;
  strncpy((char*)tmpName.name,"1.0", MAX_PARAM_NAME_LENGTH);
  loggersVerify[0] = tmpName;

  // second...
  tmpName.id = 1;
  strncpy((char*)tmpName.name, "1.1", MAX_PARAM_NAME_LENGTH);
  loggersVerify[1] = tmpName;

  // ...
  tmpName.id = 2;
  strncpy((char*)tmpName.name, "1.2", MAX_PARAM_NAME_LENGTH);
  loggersVerify[2] = tmpName;

  // ...
  tmpName.id = 3;
  strncpy((char*)tmpName.name, "2.0", MAX_PARAM_NAME_LENGTH);
  loggersVerify[3] = tmpName;

  // ...
  tmpName.id = 4;
  strncpy((char*)tmpName.name, "2.1", MAX_PARAM_NAME_LENGTH);
  loggersVerify[4] = tmpName;

  // ...
  tmpName.id = 5;
  strncpy((char*)tmpName.name, "2.2", MAX_PARAM_NAME_LENGTH);
  loggersVerify[5] = tmpName;
  // ...
  tmpName.id = 6;
  strncpy((char*)tmpName.name, "2.2.0", MAX_PARAM_NAME_LENGTH);
  loggersVerify[6] = tmpName;


  for(int i = 0; i < 7; i++)
  {
    if((loggersVerify[i].id != loggers[i].id) && strncmp((char*)loggersVerify[i].name, (char*)loggers[i].name, MAX_PARAM_NAME_LENGTH))
    {
      char tmpstr[80] = {0};
      snprintf (tmpstr, 80,"id %d with name %s not correct.\n", i, loggersVerify[i].name);
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
  }
  return result;
}

#define BUFFER_LENGTH_TEST_GET_NAME_SERIALIZED MAX_LOG_NODE_LENGTH * 16
uint8_t Log_TestGetNameSerialized(TestFw_t* obj)
{
  uint8_t result = 1;
  LogHandler_t* logHObj = LogHandler_CreateObj(2,NULL,"GetNameH",1); // Not using event handler, but we are master.
  int32_t data = 9;
  //Create a bunch of loggers...
  Log_t* logObj10 = Log_CreateObj(2,variable_type_int32, &data,NULL,logHObj,"1.0");
  Log_t* logObj11 = Log_CreateObj(2,variable_type_int32, &data,logObj10,NULL,"1.1");
  Log_t* logObj12 = Log_CreateObj(2,variable_type_int32, &data,logObj10,NULL,"1.2");


  Log_t* logObj20 = Log_CreateObj(2,variable_type_int32, &data,NULL,logHObj,"2.0");
  Log_t* logObj21 = Log_CreateObj(2,variable_type_int32, &data,logObj20,NULL,"2.1");
  Log_t* logObj22 = Log_CreateObj(2,variable_type_int32, &data,logObj20,NULL,"2.2");
  Log_t* logObj220 = Log_CreateObj(2,variable_type_int32, &data,logObj22,NULL,"2.2.0");


  if(!logObj10 || !logObj11 || !logObj12 || !logObj20 || !logObj21 || !logObj22 || !logObj220)
  {
    char tmpstr[40] = {0};
    snprintf (tmpstr, 40,"Could not create log objects.\n");
    TestFW_Report(obj, tmpstr);
    return 0;
  }

  logNameQueueItems_t logItems = {0};

  logItems.logNames = pvPortMalloc(sizeof(logNames_t) * 7);
  logItems.xMutex = xSemaphoreCreateMutex();
  logItems.nrLogNames = 0;
  if(!logItems.xMutex || !logItems.logNames)
  {
    return 0;
  }
  result &= LogHandler_GetMapping(logHObj, logItems.logNames, 7, &logItems.nrLogNames);

  uint8_t buffer[BUFFER_LENGTH_TEST_GET_NAME_SERIALIZED];
  buffer[0] = '\0';
  result &= LogHandler_AppendNodeString(&logItems, buffer, BUFFER_LENGTH_TEST_GET_NAME_SERIALIZED);

  uint8_t verify[7][MAX_LOG_NODE_LENGTH] = {};
  strncpy((char*)verify[0], "1.0[0]", MAX_LOG_NODE_LENGTH);
  strncpy((char*)verify[1], "1.1[1]", MAX_LOG_NODE_LENGTH);
  strncpy((char*)verify[2], "1.2[2]", MAX_LOG_NODE_LENGTH);
  strncpy((char*)verify[3], "2.0[3]", MAX_LOG_NODE_LENGTH);
  strncpy((char*)verify[4], "2.1[4]", MAX_LOG_NODE_LENGTH);
  strncpy((char*)verify[5], "2.2[5]", MAX_LOG_NODE_LENGTH);
  strncpy((char*)verify[6], "2.2.0[6]", MAX_LOG_NODE_LENGTH);

  uint8_t found[7] = {0};
  uint8_t* bufferPtr = buffer;

  for(int i = 0; i < logItems.nrLogNames; i++)
  {
    uint8_t moduleBuffer[MAX_LOG_NODE_LENGTH] = {0};
    moduleBuffer[0] = '\0';
    bufferPtr = FcString_GetModuleString(moduleBuffer, MAX_LOG_NODE_LENGTH, bufferPtr, BUFFER_LENGTH_TEST_GET_NAME_SERIALIZED);
    for(int j = 0; j < 7; j++)
    {
      if(0 == strncmp((char*)verify[j], (char*)moduleBuffer, MAX_LOG_NODE_LENGTH))
      {
        found[i] = 1;
        break;
      }
    }
  }
  for(int i = 0; i < logItems.nrLogNames; i++)
  {
    result &= found[i];
  }

  return result;
}
