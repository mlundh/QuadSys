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
#include "Messages/inc/Msg_Log.h"
#include "Test/Log/log_tester.h"
#include "Log/inc/logMaster.h"
#include "Log/src/log_private.h" // using private interface to be able to access the struct.
#include "Utilities/inc/string_utils.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#define SHORT_MSG (50)
#define LONG_MSG (140)

void Log_GetTCs(TestFw_t* obj)
{
    TestFW_RegisterTest(obj, "Log C Handler", LogHandler_TestCreate);
    TestFW_RegisterTest(obj, "Log C Obj", Log_TestCreate);
    TestFW_RegisterTest(obj, "Log Set Child", Log_TestSetChild);
    TestFW_RegisterTest(obj, "Log Report", Log_TestReport);
    TestFW_RegisterTest(obj, "Log Test Multiple Loggers", Log_TestMultipleLoggers);
    TestFW_RegisterTest(obj, "Log Test Multiple Handlers", Log_TestMultipleHandlers);
}

uint8_t Log_NameCB(eventHandler_t* obj, void* data, moduleMsg_t* msg);


uint8_t LogHandler_TestCreate(TestFw_t* obj)
{
    paramHander_t* paramHandler = ParamHandler_CreateObj(1, NULL,"paramM");
    eventHandler_t* evH= Event_CreateHandler(FC_Log_e, 1);

    size_t heapSize = xPortGetFreeHeapSize();
    LogMaster_t* logMaster = LogMaster_CreateObj(evH);

    uint8_t result = 1;
    if(!logMaster)
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Could not create a logHandler object.\n");
        TestFW_Report(obj, tmpstr);
        result = 0;
    }
    size_t heapSizeAfterM= xPortGetFreeHeapSize();
    size_t usedHeapM = heapSize - heapSizeAfterM;
    {

        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"LogMaster uses %d bytes of heap.\n", usedHeapM);
        TestFW_Report(obj, tmpstr);
    }
    LogHandler_t* logHObjS = LogHandler_CreateObj(2,evH, ParamHandler_GetParam(paramHandler),"TestRoot"); 
    if(!logHObjS)
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Could not create a logHandler object.\n");
        TestFW_Report(obj, tmpstr);
        result = 0;
    }
    size_t heapSizeAfterS = xPortGetFreeHeapSize();
    size_t usedHeapS = heapSizeAfterM - heapSizeAfterS;
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"LogHandler slave uses %d bytes of heap.\n", usedHeapS);
        TestFW_Report(obj, tmpstr);
    }
    Event_DeleteHandler(evH);
    ParamHandler_DeleteHandler(paramHandler);
    LogMaster_deleteHandler(logMaster);
    LogHandler_deleteHandler(logHObjS);

    return result;

}


uint8_t Log_TestCreate(TestFw_t* obj)
{
    paramHander_t* paramHandler = ParamHandler_CreateObj(1, NULL,"paramM");
    eventHandler_t* evH= Event_CreateHandler(FC_Log_e, 1);

    LogHandler_t* logHObj = LogHandler_CreateObj(2, evH, ParamHandler_GetParam(paramHandler), "TestH");

    size_t heapSize = xPortGetFreeHeapSize();
    Log_t* logObj = Log_CreateObj(2,variable_type_NoType, NULL,NULL,logHObj,"TestO");
    size_t heapSizeAfterLog = xPortGetFreeHeapSize();
    size_t usedHeapLog = heapSize - heapSizeAfterLog;
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"A log obj uses %d bytes of heap.\n", usedHeapLog);
        TestFW_Report(obj, tmpstr);
    }
    uint8_t result = 1;
    if(!logObj)
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Could not create a log object.\n");
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
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Log object not correctly initiated.\n");
        TestFW_Report(obj, tmpstr);
    }
    Event_DeleteHandler(evH);
    ParamHandler_DeleteHandler(paramHandler);
    LogHandler_deleteHandler(logHObj);
    Log_DeleteObj(logObj);

    return result;
}

uint8_t Log_TestSetChild(TestFw_t* obj)
{
    uint8_t result = 1;
    eventHandler_t* evH= Event_CreateHandler(FC_Log_e, 1);

    paramHander_t* paramHandler = ParamHandler_CreateObj(1, evH,"paramM");

    LogHandler_t* logHObj = LogHandler_CreateObj(2,evH,ParamHandler_GetParam(paramHandler),"TestH"); 
    Log_t* logObj = Log_CreateObj(2,variable_type_NoType, NULL,NULL,logHObj,"root");
    Log_t* child = Log_CreateObj(0,variable_type_NoType, NULL,logObj,NULL,"root");

    if(!logObj || !child)
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Could not create a log object.\n");
        TestFW_Report(obj, tmpstr);
        result = 0;
    }

    if(logObj->children[0] != child)
    {
        result = 0;
    }
    Event_DeleteHandler(evH);
    ParamHandler_DeleteHandler(paramHandler);
    LogHandler_deleteHandler(logHObj);
    Log_DeleteObj(logObj);
    Log_DeleteObj(child);
    return result;
}


uint8_t Log_TestReport(TestFw_t* obj)
{
    uint8_t result = 1;
    eventHandler_t* evH= Event_CreateHandler(FC_Dbg_e, 0);
    eventHandler_t* evM= Event_CreateHandler(FC_Log_e, 1);

    Event_InitHandler(evM, evH);


    paramHander_t* paramHandler = ParamHandler_CreateObj(1, NULL,"paramM");

    LogHandler_t* logHObj = LogHandler_CreateObj(2,evH,ParamHandler_GetParam(paramHandler),"Report");
    LogMaster_t* logMaster = LogMaster_CreateObj(evM);

    while(Event_Receive(evM,2));
    while(Event_Receive(evH,2));
    while(Event_Receive(evM,2));

    int32_t data = 9;
    Log_t* logObj = Log_CreateObj(2,variable_type_int32, &data,NULL,logHObj,"root1");
    if(!logObj)
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Could not create a log object.\n");
        TestFW_Report(obj, tmpstr);
        return 0;
    }
    // set the log level
    uint8_t string[] = "/paramM/Report/root1[1]/\0";
    ParamHandler_SetFromRoot(paramHandler,string ,strlen((const char*)string));

    uint32_t time = (uint32_t)xTaskGetTickCount();
    uint8_t report_res = Log_Report(logObj);

    Event_Receive(evM,2);
    Event_Receive(evM,2);
    
    if(!report_res)
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Could not create a log entry..\n");
        TestFW_Report(obj, tmpstr);
        result = 0;
    }

    if(result && report_res)
    {
        logEntry_t entry[5] = {0};
        uint32_t nrLogs = 0;
        if(!LogMaster_Getlogs(logMaster,entry,5,&nrLogs))
        {
            return 0;
        }
        logEntry_t* entryP = entry;
        if(entryP->data != 9 ||
                entryP->id != 0 ||
                entryP->time != time )
        {
            char tmpstr[LONG_MSG] = {0};
            snprintf (tmpstr, LONG_MSG,"First log entry not correct. \n Expected data: %d, id: %u, time: %lu\n Got: data: %ld, id: %lu, time: %lu\n", 9, 0, time, entryP->data, entryP->id, entryP->time);
            TestFW_Report(obj, tmpstr);
            result = 0;
        }
    }


    time = (uint32_t)xTaskGetTickCount();
    data = 879;
    report_res = Log_Report(logObj);

    Event_Receive(evM,2);
    Event_Receive(evM,2);
    
    if(!report_res)
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Could not create a log entry.\n");
        TestFW_Report(obj, tmpstr);
        result = 0;
    }
    

    if(result && report_res)
    {
        logEntry_t entry[5] = {0};
        uint32_t nrLogs = 0;
        if(!LogMaster_Getlogs(logMaster,entry,5,&nrLogs))
        {
            return 0;
        }
        logEntry_t* entryP = entry;
        if(entryP->data != 879 ||
                entryP->id != 0 ||
                entryP->time != time )
        {
            char tmpstr[LONG_MSG] = {0};
            snprintf (tmpstr, LONG_MSG,"Second log entry not correct. \n Expected data: %d, id: %u, time: %lu\n Got: data: %ld, id: %lu, time: %lu\n", 879, 0, time, entryP->data, entryP->id, entryP->time);
            TestFW_Report(obj, tmpstr);
            result = 0;
        }
    }
    Event_DeleteHandler(evH);
    Event_DeleteHandler(evM);
    ParamHandler_DeleteHandler(paramHandler);
    LogHandler_deleteHandler(logHObj);
    LogMaster_deleteHandler(logMaster);
    Log_DeleteObj(logObj);
    return result;
}

uint8_t Log_TestMultipleLoggers(TestFw_t* obj)
{
    eventHandler_t* evH= Event_CreateHandler(FC_Dbg_e, 0);
    eventHandler_t* evM= Event_CreateHandler(FC_Log_e, 1);

    Event_InitHandler(evM, evH);

    uint8_t result = 1;
    LogHandler_t* logHObj = LogHandler_CreateObj(2,evH,NULL,"Report1"); // Not using event handler, but we are master.
    LogMaster_t* logMaster = LogMaster_CreateObj(evM);

    while(Event_Receive(evM,2));
    while(Event_Receive(evH,2));
    while(Event_Receive(evM,2));

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
    uint8_t string[] = "Report1/root1[1]/\0";
    Param_SetFromHere(LogHandler_GetParameter(logHObj),string ,strlen((const char*)string));
    uint8_t string2[] = "Report1/root1/child1[1]/\0";
    Param_SetFromHere(LogHandler_GetParameter(logHObj),string2 ,strlen((const char*)string2));

    uint32_t time = (uint32_t)xTaskGetTickCount();
    uint8_t report_res = Log_Report(logObj);
    report_res &= Log_Report(logObj2);

    while(Event_Receive(evM,2));


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
        if(!LogMaster_Getlogs(logMaster,entry,5,&nrLogs))
        {
            return 0;
        }
        logEntry_t* entryP = entry;
        if(entryP->data != data ||
                entryP->id != 0 ||
                entryP->time != time )
        {
            char tmpstr[LONG_MSG] = {0};
            snprintf (tmpstr, LONG_MSG,"First log entry not correct. \n Expected data: %d, id: %u, time: %lu\n Got: data: %ld, id: %lu, time: %lu\n", 9, 0, time, entryP->data, entryP->id, entryP->time);
            TestFW_Report(obj, tmpstr);
            result = 0;
        }
        entryP++;
        if(entryP->data != data2 ||
                entryP->id != 1 ||
                entryP->time != time )
        {
            char tmpstr[LONG_MSG] = {0};
            snprintf (tmpstr, LONG_MSG,"Second log entry not correct. \n Expected data: %d, id: %u, time: %lu\n Got: data: %ld, id: %lu, time: %lu\n", 9, 0, time, entryP->data, entryP->id, entryP->time);
            TestFW_Report(obj, tmpstr);
            result = 0;
        }
    }
    Event_DeleteHandler(evH);
    Event_DeleteHandler(evM);
    LogHandler_deleteHandler(logHObj);
    LogMaster_deleteHandler(logMaster);
    Log_DeleteObj(logObj);
    Log_DeleteObj(logObj2);

    return result;
}



uint8_t Log_TestMultipleHandlers(TestFw_t* obj)
{
    eventHandler_t* evH= Event_CreateHandler(FC_Dbg_e, 0);
    eventHandler_t* evM= Event_CreateHandler(FC_Log_e, 1);
    
    Event_InitHandler(evM, evH);


    uint8_t result = 1;
    LogHandler_t* logHObjT1 = LogHandler_CreateObj(2,evM,NULL,"TH1");
    int32_t data = 9;
    Log_t* logObjT1 = Log_CreateObj(2,variable_type_int32, &data,NULL,logHObjT1,"LogT1");

    LogHandler_t* logHObjT2 = LogHandler_CreateObj(2,evH,NULL,"TH2"); 
    int32_t data2 = 800;
    Log_t* logObjT2 = Log_CreateObj(2,variable_type_int32, &data2,NULL,logHObjT2,"LogT2");
    LogMaster_t* logMaster = LogMaster_CreateObj(evM);

    if(!logObjT1 || !logObjT2 || !logHObjT1 ||! logHObjT2 ||!logMaster)
    {
        char tmpstr[40] = {0};
        snprintf (tmpstr, 40,"Could not create log objects.\n");
        TestFW_Report(obj, tmpstr);
        return 0;
    }

    while(Event_Receive(evM,2));
    while(Event_Receive(evH,2));
    while(Event_Receive(evM,2));

    // set the log level
    uint8_t string[] = "TH1/LogT1[1]/\0";
    Param_SetFromHere(LogHandler_GetParameter(logHObjT1),string ,strlen((const char*)string));
    uint8_t string2[] = "TH2/LogT2[1]/\0";
    Param_SetFromHere(LogHandler_GetParameter(logHObjT2),string2 ,strlen((const char*)string2));


    uint32_t time = (uint32_t)xTaskGetTickCount();
    uint8_t report_res = Log_Report(logObjT1);
    report_res &= Log_Report(logObjT1);
    report_res &= Log_Report(logObjT2);
    report_res &= Log_Report(logObjT2);


    while(Event_Receive(evH,2));
    while(Event_Receive(evM,2));

    if(!report_res)
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Could not create a log entry..\n");
        TestFW_Report(obj, tmpstr);
        result = 0;
    }
    if(result && report_res)
    {
        logEntry_t entry[5] = {0};
        uint32_t nrLogs = 0;
        if(!LogMaster_Getlogs(logMaster,entry,5,&nrLogs))
        {
            return 0;
        }
        logEntry_t* entryP = entry;
        if(entryP->data != data ||
                entryP->id != 0 ||
                entryP->time != time )
        {
            char tmpstr[LONG_MSG] = {0};
            snprintf (tmpstr, LONG_MSG,"First log entry not correct. \n Expected data: %d, id: %u, time: %lu\n Got: data: %ld, id: %lu, time: %lu\n", 9, 0, time, entryP->data, entryP->id, entryP->time);
            TestFW_Report(obj, tmpstr);
            result = 0;
        }
        entryP++;
        if(entryP->data != data ||
                entryP->id != 0 ||
                entryP->time != time )
        {
            char tmpstr[LONG_MSG] = {0};
            snprintf (tmpstr, LONG_MSG,"Second log entry not correct. \n Expected data: %d, id: %u, time: %lu\n Got: data: %ld, id: %lu, time: %lu\n", 9, 0, time, entryP->data, entryP->id, entryP->time);
            TestFW_Report(obj, tmpstr);
            result = 0;
        }
        entryP++;
        if(entryP->data != data2 ||
                entryP->id != MAX_LOGGERS_PER_HANDLER ||
                entryP->time != time )
        {
            char tmpstr[LONG_MSG] = {0};
            snprintf (tmpstr, LONG_MSG,"Third log entry not correct. \n Expected data: %d, id: %u, time: %lu\n Got: data: %ld, id: %lu, time: %lu\n", 9, 0, time, entryP->data, entryP->id, entryP->time);
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
    Event_DeleteHandler(evH);
    Event_DeleteHandler(evM);


    LogHandler_deleteHandler(logHObjT1);
    LogHandler_deleteHandler(logHObjT2);

    LogMaster_deleteHandler(logMaster);

    Log_DeleteObj(logObjT1);
    Log_DeleteObj(logObjT2);

    return result;
}
