/*
 * AppLogTest.c
 *
 *  Created on: Aug 15, 2019
 *      Author: mlundh
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

#include "../AppLogTest.h"
#include "Components/AppLog/inc/AppLog.h"
#include "Components/AppLog/src/AppLogHandlerPrivate.h"
#include "Messages/inc/Msg_AppLog.h"
#include "Components/AppLog/inc/AppLogHandler.h"
#include <stddef.h>
#include <string.h>


uint8_t AppLogHandler_TestCreate(TestFw_t* obj);
uint8_t AppLogHandler_TestReport(TestFw_t* obj);
uint8_t AppLogHandler_TestReportMacro(TestFw_t* obj);


void AppLog_GetTCs(TestFw_t* obj)
{
    TestFW_RegisterTest(obj, "AppLog Create", AppLogHandler_TestCreate);
    TestFW_RegisterTest(obj, "AppLog Report", AppLogHandler_TestReport);
    TestFW_RegisterTest(obj, "AppLog ReportM", AppLogHandler_TestReportMacro);


}
#define SHORT_MSG (200)
#define LONG_MSG (500)

uint8_t AppLogHandler_TestCreate(TestFw_t* obj)
{
    AppLogHandler_t* logobj = AppLogHandler_Create(NULL);

    uint8_t result = 1;
    if(!logobj)
    {
        TEST_REPORT_ERROR(obj,"Could not create a AppLogHandler object." )
        result = 0;
    }
    if(!result ||
            logobj->evHandler || //Initialized to null...
            !logobj->backend )
    {
        TEST_REPORT_ERROR(obj,"AppLogHandler object not correctly initiated." )
        result = 0;
    }
    AppLogHandler_Delete(logobj);
    return result;
}

uint8_t AppLogHandler_TestReport(TestFw_t* obj)
{
    eventHandler_t* evHandlerAppLog = Event_CreateHandler(FC_SerialIOrx_e, 1);
    eventHandler_t* evHandlerTester = Event_CreateHandler(FC_Led_e, 0);
    AppLogHandler_t* logobj = AppLogHandler_Create(evHandlerAppLog);

    Event_InitHandler(evHandlerAppLog, evHandlerTester);

    while(Event_Receive(evHandlerAppLog,2)){}
    while(Event_Receive(evHandlerTester,2)){}

    uint8_t result = 1;
    if(!logobj || !evHandlerAppLog || !evHandlerTester)
    {
        TEST_REPORT_ERROR(obj,"Could not create a AppLogHandler or eventHandler object(s)." )
        result = 0;
    }
    // TODO the desitnation address should not be known by the module.
    moduleMsg_t* msg = Msg_AppLogCreate(FC_SerialIOrx_e,0,writeAppLog,SHORT_MSG); 

    uint8_t bufferOrig[SHORT_MSG] = "TestingTesting this is from the appLog.\0";
    uint8_t* ptr = Msg_AppLogGetPayload(msg);
    uint32_t buffSize = strlen((char*)bufferOrig);

    Msg_AppLogSetPayloadlength(msg, buffSize);
    memcpy((char*)ptr,bufferOrig, buffSize);

    Event_Send(evHandlerTester, msg);

    //We are only using one thread, so now we have to poll the receiver.
    Event_Receive(evHandlerAppLog, 2);

    uint8_t buffer[SHORT_MSG] = {0};
    AppLogBackend_GetLog(logobj->backend, buffer, SHORT_MSG);
    
   if(strncmp((char*)bufferOrig, (char*)buffer,buffSize) == 0)
    {
        result &= 1;
    }
    else
    {
        TEST_REPORT_ERROR(obj, "expected: \"%s\" \ngot: \"%s\"!.", (char*)bufferOrig, (char*)buffer)
        result = 0;
    }

    AppLogHandler_Delete(logobj);
    Event_DeleteHandler(evHandlerAppLog);
    Event_DeleteHandler(evHandlerTester);
    return result;
}


uint8_t AppLogHandler_TestReportMacro(TestFw_t* obj)
{
    eventHandler_t* evHandlerAppLog = Event_CreateHandler(FC_AppLog_e, 1);
    eventHandler_t* evHandlerTester = Event_CreateHandler(FC_Led_e, 0);
    AppLogHandler_t* logobj = AppLogHandler_Create(evHandlerAppLog);

    Event_InitHandler(evHandlerAppLog, evHandlerTester);

    while(Event_Receive(evHandlerAppLog,2)){}
    while(Event_Receive(evHandlerTester,2)){}
    
    uint8_t result = 1;
    if(!logobj || !evHandlerAppLog || !evHandlerTester)
    {
        TEST_REPORT_ERROR(obj, "Could not create a AppLogHandler or eventHandler object(s).");
        result = 0;
    }
    char orig[] = {"hello!"};
    LOG_ENTRY(evHandlerTester, "%s", orig);

    char expected[LOG_LENGTH_MAX] = {};

    snprintf(expected, LOG_LENGTH_MAX, "%8lu: %s\n", xTaskGetTickCount(),orig);

    //We are only using one thread, so now we have to poll the receiver.
    Event_Receive(evHandlerAppLog, 2);

    uint8_t buffer[SHORT_MSG] = {0};
    AppLogBackend_GetLog(logobj->backend, buffer, SHORT_MSG);
    
   if(strncmp(expected, (char*)buffer,8) == 0)
    {
        result &= 1;
    }
    else
    {
        TEST_REPORT_ERROR(obj, "expected: \"%s\" \ngot: \"%s\"!.", expected, (char*)buffer);
        result = 0;
    }

    AppLogHandler_Delete(logobj);
    Event_DeleteHandler(evHandlerAppLog);
    Event_DeleteHandler(evHandlerTester);
    return result;
}
