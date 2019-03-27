/*
 * ParameterTester.c
 *
 * Copyright (C) 2016 Martin Lundh
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
#include "../parameter_tester.h"
#include "Components/Parameters/inc/parameters.h"
#include "Modules/EventHandler/inc/event_handler.h"
#include "Components/Parameters/inc/paramHandler.h"
#include "Modules/Messages/inc/Msg_Param.h"
#include <string.h>

typedef struct
{
    eventHandler_t* evHandlerMaster;
    eventHandler_t* evHandler;
    paramHander_t*  paramHandler;
    param_obj_t*    param1;
    param_obj_t*    param11;
    param_obj_t*    param12;
    int32_t         value1;
    int32_t         value11;
    int32_t         value12;
    uint8_t*        response;
    uint32_t        responseSize;
    uint8_t         done;

}ParamTest_t;

#define REPORT_LONG (200)
#define PAYLOAD_LENGTH (150)

uint8_t ParamT_HandleGetParam(eventHandler_t* obj, void* data, moduleMsg_t* msg);
ParamTest_t* ParamT_Initialize();
void ParamT_Delete(ParamTest_t* obj);

uint8_t ParamT_TestGetOneHandler(TestFw_t* Tobj);
uint8_t ParamT_TestGetTwoOneHandler(TestFw_t* Tobj);


void ParamT_GetTCs(TestFw_t* Tobj)
{
    TestFW_RegisterTest(Tobj, "ParamGet", ParamT_TestGetOneHandler);
    TestFW_RegisterTest(Tobj, "ParamGet2", ParamT_TestGetTwoOneHandler);

}



ParamTest_t* ParamT_Initialize(uint32_t responseSize)
{
    ParamTest_t* obj = pvPortMalloc(sizeof(ParamTest_t));
    obj->evHandler = Event_CreateHandler(FC_Param_e, 0);
    obj->evHandlerMaster = Event_CreateHandler(GS_Param_e, 1);
    obj->paramHandler = ParamHander_CreateObj(1, obj->evHandler,"paramH", 1);
    obj->value11 = 0;
    obj->value12 = 0;
    obj->param1 = Param_CreateObj(2,variable_type_int32, readWrite, &obj->value1,"param1",
            ParamHandler_GetParam(obj->paramHandler), NULL);
    obj->param11 = Param_CreateObj(0,variable_type_int32, readWrite, &obj->value11,"param11",
            obj->param1, NULL);
    obj->param12 = Param_CreateObj(0,variable_type_int32, readWrite, &obj->value12,"param12",
            obj->param1, NULL);
    obj->response = pvPortMalloc(responseSize);
    obj->response[0] = '\0';
    obj->responseSize = responseSize;
    obj->done = 0;
    Event_RegisterCallback(obj->evHandlerMaster, Msg_Param_e, ParamT_HandleGetParam, obj);
    Event_InitHandler(obj->evHandlerMaster, obj->evHandler);
    return obj;
}

void ParamT_Delete(ParamTest_t* obj)
{
    Event_DeleteHandler(obj->evHandler);
    Event_DeleteHandler(obj->evHandlerMaster);
    ParamHander_deleteHandler(obj->paramHandler);
    Param_DeleteObj(obj->param1);
    Param_DeleteObj(obj->param11);
    Param_DeleteObj(obj->param12);
    vPortFree(obj->response);
    vPortFree(obj);
}

uint8_t ParamT_TestGetOneHandler(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    ParamTest_t* obj = ParamT_Initialize(PAYLOAD_LENGTH);

    uint8_t payload[PAYLOAD_LENGTH];
    moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, payload, 0, PAYLOAD_LENGTH);
    Event_Send(obj->evHandlerMaster, msg);

    //We are only using one thread, so now we have to poll the receiver.
    Event_Receive(obj->evHandler, 2);

    //This should have generated a response, poll the sender.
    Event_Receive(obj->evHandlerMaster, 2);

    //The response should have populated the payload, make sure it has done so in a correct way.
    uint8_t result = 0;
    char expected[PAYLOAD_LENGTH] = "/paramH<0>/param1<6>[0]/param11<6>[0]/../param12<6>[0]/../../";
    if(strncmp((char*)obj->response, expected,PAYLOAD_LENGTH) == 0)
    {
        result = 1;
    }
    else
    {
        char tmpstr[REPORT_LONG] = {0};
        snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
        TestFW_Report(Tobj, tmpstr);
    }

    ParamT_Delete(obj);
    return result;

}

#define PAYLOAD_LENGTH_SHORT (60)
uint8_t ParamT_TestGetTwoOneHandler(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    uint8_t result = 1;
    ParamTest_t* obj = ParamT_Initialize(PAYLOAD_LENGTH_SHORT);
    {
        uint8_t payload[PAYLOAD_LENGTH];
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, payload, 0, PAYLOAD_LENGTH_SHORT);
        Event_Send(obj->evHandlerMaster, msg);

        //We are only using one thread, so now we have to poll the receiver.
        Event_Receive(obj->evHandler, 2);

        //This should have generated a response, poll the sender.
        Event_Receive(obj->evHandlerMaster, 2);

        //The response should have populated the payload, make sure it has done so in a correct way.
        char expected[PAYLOAD_LENGTH_SHORT] = "/paramH<0>/param1<6>[0]/param11<6>[0]/../";
        if(strncmp((char*)obj->response, expected,PAYLOAD_LENGTH_SHORT) == 0)
        {
            result &= 1;
        }
        else
        {
            result = 0;
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
        }

        if(obj->done)
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Dump unexpectedly finished.\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
    }
    {
        uint8_t payload[PAYLOAD_LENGTH];
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, payload, 0, PAYLOAD_LENGTH_SHORT);
        Event_Send(obj->evHandlerMaster, msg);

        //We are only using one thread, so now we have to poll the receiver.
        Event_Receive(obj->evHandler, 2);

        //This should have generated a response, poll the sender.
        Event_Receive(obj->evHandlerMaster, 2);

        //The response should have populated the payload, make sure it has done so in a correct way.
        char expected[PAYLOAD_LENGTH_SHORT] = "/paramH<0>/param1<6>[0]/param12<6>[0]/../../";
        if(strncmp((char*)obj->response, expected,PAYLOAD_LENGTH) == 0)
        {
            result &= 1;
        }
        else
        {
            result = 0;
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
        }
        if(!obj->done)
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Dump unexpectedly not finished.\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
    }
    ParamT_Delete(obj);
    return result;

}

uint8_t ParamT_HandleGetParam(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    ParamTest_t* testObj = (ParamTest_t*)data;
    // This is not the proper way of doing this, no need to copy the playload, here it is done for simplicity.
    memcpy(testObj->response, Msg_ParamGetPayload(msg), testObj->responseSize);
    testObj->done = Msg_ParamGetLastinsequence(msg) ? 1 : 0;
    // TODO do we need this?
    return 1;
}

