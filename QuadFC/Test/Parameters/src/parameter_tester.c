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
#include "Parameters/inc/parameters.h"
#include "EventHandler/inc/event_handler.h"
#include "Components/Parameters/inc/paramHandler.h"
#include "Messages/inc/Msg_Param.h"
#include <string.h>

typedef struct
{
    eventHandler_t* evHandlerMaster;
    eventHandler_t* evHandler;
    eventHandler_t* evHandlerTester;
    paramHander_t*  paramHandler;
    paramHander_t*  paramHandlerMaster;

    param_obj_t*    param1;
    param_obj_t*    param11;
    param_obj_t*    param12;
    int32_t         value1;
    int32_t         value11;
    int32_t         value12;

    param_obj_t*    param2;
    param_obj_t*    param21;
    param_obj_t*    param22;
    int32_t         value2;
    int32_t         value21;
    int32_t         value22;

    uint8_t*        response;
    uint32_t        responseSize;
    uint32_t        receivedSize;
    uint8_t         done;
    uint8_t         sequenceNr;
    uint8_t         lastInSequence;

}ParamTestTwoHandlers_t;

typedef struct
{
    eventHandler_t* evHandlerMaster;
    eventHandler_t* evHandlerTester;
    paramHander_t*  paramHandlerMaster;

    param_obj_t*    param1;
    param_obj_t*    param11;
    param_obj_t*    param12;
    int32_t         value1;
    int32_t         value11;
    int32_t         value12;

    uint8_t*        response;
    uint32_t        responseSize;
    uint8_t         done;

}ParamTestOneHandler_t;

#define REPORT_LONG (200)
#define PAYLOAD_LENGTH (150)
#define PAYLOAD_LENGTH_SHORT (60)
#define PAYLOAD_LENGTH_LONG (250)

uint8_t ParamT_HandleGetParam(eventHandler_t* obj, void* data, moduleMsg_t* msg);
uint8_t ParamT_HandleGetParamOneHandler(eventHandler_t* obj, void* data, moduleMsg_t* msg);


ParamTestTwoHandlers_t* ParamT_InitializeTwoHandlers();
void ParamT_DeleteTwoHandlers(ParamTestTwoHandlers_t* obj);

uint8_t ParamT_TestGetOneHandler(TestFw_t* Tobj);
uint8_t ParamT_TestGetTwoOneHandler(TestFw_t* Tobj);
uint8_t ParamT_TestSetGetOneHandler(TestFw_t* Tobj);
uint8_t ParamT_TestSetGetOneHandlerSearch(TestFw_t* Tobj);
uint8_t ParamT_TestSaveLoadOneHandler(TestFw_t* Tobj);
uint8_t ParamT_TestGetTwoHandlers(TestFw_t* Tobj);
uint8_t ParamT_TestGetShortTwoHandlers(TestFw_t* Tobj);
uint8_t ParamT_TestSetGetTwoHandlers(TestFw_t* Tobj);



void ParamT_GetTCs(TestFw_t* Tobj)
{
    TestFW_RegisterTest(Tobj, "Param Get", ParamT_TestGetOneHandler);
    //TestFW_RegisterTest(Tobj, "GetShortMsg", ParamT_TestGetTwoOneHandler);// Does not work. Set internal length of messages in param handler to 60 to verify function.
    TestFW_RegisterTest(Tobj, "Param SetGet", ParamT_TestSetGetOneHandler);
    TestFW_RegisterTest(Tobj, "Param SetGetSearch", ParamT_TestSetGetOneHandlerSearch);
    TestFW_RegisterTest(Tobj, "Param SaveLoad", ParamT_TestSaveLoadOneHandler);
    TestFW_RegisterTest(Tobj, "Param GetTwoHandlers", ParamT_TestGetTwoHandlers);
    //TestFW_RegisterTest(Tobj, "GetShortTwoHandlers", ParamT_TestGetShortTwoHandlers); // Does not work. Set internal length of messages in param handler to 60 to verify function.
    TestFW_RegisterTest(Tobj, "Param SetGetTwoHandlers", ParamT_TestSetGetTwoHandlers);

}



ParamTestTwoHandlers_t* ParamT_InitializeTwoHandlers(uint32_t responseSize)
{
    ParamTestTwoHandlers_t* obj = pvPortMalloc(sizeof(ParamTestTwoHandlers_t));
    obj->evHandler = Event_CreateHandler(FC_Log_e, 0);
    obj->evHandlerMaster = Event_CreateHandler(FC_Param_e, 1);
    obj->evHandlerTester = Event_CreateHandler(FC_Dbg_e, 0);

    obj->paramHandler = ParamHandler_CreateObj(1, obj->evHandler,"paramH", 0);
    obj->paramHandlerMaster = ParamHandler_CreateObj(1, obj->evHandlerMaster,"paramHM", 1);

    obj->value1  = 0;
    obj->value11 = 0;
    obj->value12 = 0;
    obj->param1 = Param_CreateObj(2,variable_type_int32, readWrite, &obj->value1,"param1",
            ParamHandler_GetParam(obj->paramHandler));
    obj->param11 = Param_CreateObj(0,variable_type_int32, readWrite, &obj->value11,"param11",
            obj->param1);
    obj->param12 = Param_CreateObj(0,variable_type_int32, readWrite, &obj->value12,"param12",
            obj->param1);

    obj->value2  = 0;
    obj->value21 = 0;
    obj->value22 = 0;
    obj->param2 = Param_CreateObj(2,variable_type_int32, readWrite, &obj->value2,"param2",
            ParamHandler_GetParam(obj->paramHandlerMaster));
    obj->param21 = Param_CreateObj(0,variable_type_int32, readWrite, &obj->value21,"param21",
            obj->param2);
    obj->param22 = Param_CreateObj(0,variable_type_int32, readWrite, &obj->value22,"param22",
            obj->param2);

    obj->response = pvPortMalloc(responseSize);
    obj->response[0] = '\0';
    obj->responseSize = responseSize;
    obj->done = 0;
    obj->sequenceNr = 0;
    obj->lastInSequence = 0;
    Event_RegisterCallback(obj->evHandlerTester, Msg_Param_e, ParamT_HandleGetParam, obj);
    Event_InitHandler(obj->evHandlerMaster, obj->evHandler);
    Event_InitHandler(obj->evHandlerMaster, obj->evHandlerTester);

    ParamHandler_InitMaster(obj->paramHandlerMaster);
    Event_Receive(obj->evHandler,2);
    Event_Receive(obj->evHandlerTester,2);
    Event_Receive(obj->evHandlerMaster,2);


    return obj;
}


ParamTestOneHandler_t* ParamT_InitializeOneHandler(uint32_t responseSize)
{
    ParamTestOneHandler_t* obj = pvPortMalloc(sizeof(ParamTestOneHandler_t));
    obj->evHandlerMaster = Event_CreateHandler(FC_Param_e, 1);
    obj->evHandlerTester = Event_CreateHandler(FC_Log_e, 0);

    obj->paramHandlerMaster = ParamHandler_CreateObj(1, obj->evHandlerMaster,"paramHM", 1);

    obj->value1  = 0;
    obj->value11 = 0;
    obj->value12 = 0;
    obj->param1 = Param_CreateObj(2,variable_type_int32, readWrite, &obj->value1,"param1",
            ParamHandler_GetParam(obj->paramHandlerMaster));
    obj->param11 = Param_CreateObj(0,variable_type_int32, readWrite, &obj->value11,"param11",
            obj->param1);
    obj->param12 = Param_CreateObj(0,variable_type_int32, readWrite, &obj->value12,"param12",
            obj->param1);

    obj->response = pvPortMalloc(responseSize);
    obj->response[0] = '\0';
    obj->responseSize = responseSize;
    obj->done = 0;
    Event_RegisterCallback(obj->evHandlerTester, Msg_Param_e, ParamT_HandleGetParamOneHandler, obj);
    Event_InitHandler(obj->evHandlerMaster, obj->evHandlerTester);

    ParamHandler_InitMaster(obj->paramHandlerMaster);
    Event_Receive(obj->evHandlerTester,2);
    Event_Receive(obj->evHandlerMaster,2);

    return obj;
}


void ParamT_DeleteTwoHandlers(ParamTestTwoHandlers_t* obj)
{
    Event_DeleteHandler(obj->evHandler);
    Event_DeleteHandler(obj->evHandlerMaster);
    Event_DeleteHandler(obj->evHandlerTester);

    ParamHandler_DeleteHandler(obj->paramHandler);
    ParamHandler_DeleteHandler(obj->paramHandlerMaster);

    Param_DeleteObj(obj->param1);
    Param_DeleteObj(obj->param11);
    Param_DeleteObj(obj->param12);

    Param_DeleteObj(obj->param2);
    Param_DeleteObj(obj->param21);
    Param_DeleteObj(obj->param22);

    vPortFree(obj->response);
    vPortFree(obj);
    vTaskDelay(5);
}

void ParamT_DeleteOneHandler(ParamTestOneHandler_t* obj)
{
    Event_DeleteHandler(obj->evHandlerMaster);
    Event_DeleteHandler(obj->evHandlerTester);

    ParamHandler_DeleteHandler(obj->paramHandlerMaster);

    Param_DeleteObj(obj->param1);
    Param_DeleteObj(obj->param11);
    Param_DeleteObj(obj->param12);

    vPortFree(obj->response);
    vPortFree(obj);
    vTaskDelay(5);
}

uint8_t ParamT_TestGetOneHandler(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    ParamTestOneHandler_t* obj = ParamT_InitializeOneHandler(PAYLOAD_LENGTH);

    moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);

    Event_Send(obj->evHandlerTester, msg);

    //We are only using one thread, so now we have to poll the receiver.
    Event_Receive(obj->evHandlerMaster, 2);

    //This should have generated a response, poll the sender.
    Event_Receive(obj->evHandlerTester, 2);

    //The response should have populated the payload, make sure it has done so in a correct way.
    uint8_t result = 0;
    char expected[PAYLOAD_LENGTH] = "/paramHM<0>/param1<6>[0]/param11<6>[0]/../param12<6>[0]/../../";
    if(strncmp((char*)obj->response, expected,obj->responseSize) == 0)
    {
        result = 1;
    }
    else
    {
        char tmpstr[REPORT_LONG] = {0};
        snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
        TestFW_Report(Tobj, tmpstr);
    }

    ParamT_DeleteOneHandler(obj);
    return result;

}

uint8_t ParamT_TestGetTwoOneHandler(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    uint8_t result = 1;
    ParamTestOneHandler_t* obj = ParamT_InitializeOneHandler(PAYLOAD_LENGTH_SHORT);
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);

        //We are only using one thread, so now we have to poll the receiver.
        Event_Receive(obj->evHandlerMaster, 2);

        //This should have generated a response, poll the sender.
        Event_Receive(obj->evHandlerTester, 2);

        //The response should have populated the payload, make sure it has done so in a correct way.
        char expected[PAYLOAD_LENGTH_SHORT] = "/paramHM<0>/param1<6>[0]/param11<6>[0]/../";
        if(strncmp((char*)obj->response, expected,obj->responseSize) == 0)
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
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);

        //We are only using one thread, so now we have to poll the receiver.
        Event_Receive(obj->evHandlerMaster, 2);

        //This should have generated a response, poll the sender.
        Event_Receive(obj->evHandlerTester, 2);

        //The response should have populated the payload, make sure it has done so in a correct way.
        char expected[PAYLOAD_LENGTH_SHORT] = "/paramHM<0>/param1<6>[0]/param12<6>[0]/../../";
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
        if(!obj->done)
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Dump unexpectedly not finished.\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
    }
    ParamT_DeleteOneHandler(obj);
    return result;

}


uint8_t ParamT_TestSetGetOneHandler(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    ParamTestOneHandler_t* obj = ParamT_InitializeOneHandler(PAYLOAD_LENGTH);

    uint8_t payloadSet[PAYLOAD_LENGTH] = "/paramHM/param1[700]/param11[1024]/../param12[1038]/\0";

    moduleMsg_t* msgSet = Msg_ParamCreate(FC_Param_e, 0, param_set, 0, 0, PAYLOAD_LENGTH);
    uint8_t* payload = Msg_ParamGetPayload(msgSet);

    payload = memcpy(payload, payloadSet, PAYLOAD_LENGTH);
    uint32_t payloadLength = strlen((char*)payload);

    Msg_ParamSetPayloadlength(msgSet, payloadLength);

    Event_Send(obj->evHandlerTester, msgSet);

    //We are only using one thread, so now we have to poll the receiver.
    Event_Receive(obj->evHandlerMaster, 2);

    //Then we want to read back the updated parameters.
    moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, PAYLOAD_LENGTH);
    Event_Send(obj->evHandlerTester, msg);
 
    //We are only using one thread, so now we have to poll the receiver.
    Event_Receive(obj->evHandlerMaster, 2);

    //This should have generated a response, poll the sender.
    Event_Receive(obj->evHandlerTester, 2);

    //The response should have populated the payload, make sure it has done so in a correct way.
    uint8_t result = 0;
    char expected[PAYLOAD_LENGTH] = "/paramHM<0>/param1<6>[700]/param11<6>[1024]/../param12<6>[1038]/../../";
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

    ParamT_DeleteOneHandler(obj);
    return result;

}

uint8_t ParamT_TestSetGetOneHandlerSearch(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    ParamTestOneHandler_t* obj = ParamT_InitializeOneHandler(PAYLOAD_LENGTH);

    moduleMsg_t* msgSet = Msg_ParamCreate(FC_Param_e, 0, param_set, 0, 0, PAYLOAD_LENGTH);
    uint8_t* payloadMsg = Msg_ParamGetPayload(msgSet);

    {
        uint8_t payloadSet[PAYLOAD_LENGTH_LONG] = "/paramH<0>/param2<6>[48]/param21<6>[63]/../param22<6>[88]/../../../paramHM<0>/param1<6>[700]/param11<6>[1024]/../param12<6>[1038]/../../\0";
        memcpy(payloadMsg, payloadSet, PAYLOAD_LENGTH);
    }
    uint32_t payloadLength = strlen((char*)payloadMsg);

    Msg_ParamSetPayloadlength(msgSet, payloadLength);


    Event_Send(obj->evHandlerTester, msgSet);

    //We are only using one thread, so now we have to poll the receiver.
    Event_Receive(obj->evHandlerMaster, 2);

    //Then we want to read back the updated parameters.
    moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
    Event_Send(obj->evHandlerTester, msg);

    //We are only using one thread, so now we have to poll the receiver.
    Event_Receive(obj->evHandlerMaster, 2);

    //This should have generated a response, poll the sender.
    Event_Receive(obj->evHandlerTester, 2);

    //The response should have populated the payload, make sure it has done so in a correct way.
    uint8_t result = 0;
    char expected[PAYLOAD_LENGTH] = "/paramHM<0>/param1<6>[700]/param11<6>[1024]/../param12<6>[1038]/../../";
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

    ParamT_DeleteOneHandler(obj);
    return result;

}


uint8_t ParamT_TestSaveLoadOneHandler(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    ParamTestOneHandler_t* obj = ParamT_InitializeOneHandler(PAYLOAD_LENGTH);
    uint8_t result = 0;


    {
        // Write non-default values.
        moduleMsg_t* msgSet = Msg_ParamCreate(FC_Param_e, 0, param_set, 0, 0, PAYLOAD_LENGTH);
        uint8_t* payloadMsg = Msg_ParamGetPayload(msgSet);

        uint8_t payloadSet[PAYLOAD_LENGTH_LONG] = "/paramHM<0>/param1<6>[700]/param11<6>[1024]/../param12<6>[1038]/../../\0";
        memcpy(payloadMsg, payloadSet, PAYLOAD_LENGTH);
        uint32_t payloadLength = strlen((char*)payloadMsg);
        Msg_ParamSetPayloadlength(msgSet, payloadLength);
        Event_Send(obj->evHandlerTester, msgSet);
        //We are only using one thread, so now we have to poll the receiver.
        Event_Receive(obj->evHandlerMaster, 2);

    }
    {
        //Then save the parameters.
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_save, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);
        Event_Receive(obj->evHandlerMaster, 2);

    }
    // Then write new parameters.

    {
        // Write non-default values.
        moduleMsg_t* msgSet = Msg_ParamCreate(FC_Param_e, 0, param_set, 0, 0, PAYLOAD_LENGTH);
        uint8_t* payloadMsg = Msg_ParamGetPayload(msgSet);

        uint8_t payloadSet[PAYLOAD_LENGTH_LONG] = "/paramHM<0>/param1<6>[9]/param11<6>[9]/../param12<6>[9]/../../\0";
        memcpy(payloadMsg, payloadSet, PAYLOAD_LENGTH);
        uint32_t payloadLength = strlen((char*)payloadMsg);
        Msg_ParamSetPayloadlength(msgSet, payloadLength);
        Event_Send(obj->evHandlerTester, msgSet);
        Event_Receive(obj->evHandlerMaster, 2);

    }

    //Then we want to read back the updated parameters.
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);

        //We are only using one thread, so now we have to poll the receiver.
        Event_Receive(obj->evHandlerMaster, 2);

        //This should have generated a response, poll the sender.
        Event_Receive(obj->evHandlerTester, 2);
    }
    //The response should have populated the payload, make sure it has done so in a correct way.

    {
        char expected[PAYLOAD_LENGTH] = "/paramHM<0>/param1<6>[9]/param11<6>[9]/../param12<6>[9]/../../";
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
    }

    // Now load the saved parameters.
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_load, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);
        Event_Receive(obj->evHandlerMaster, 2);
        Event_Receive(obj->evHandlerMaster, 2);

    }

    //Then we want to read back the loaded parameters.
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);

        //We are only using one thread, so now we have to poll the receiver.
        Event_Receive(obj->evHandlerMaster, 2);

        //This should have generated a response, poll the sender.
        Event_Receive(obj->evHandlerTester, 2);
    }
    //The response should have populated the payload, make sure it has done so in a correct way.
    {
        char expected[PAYLOAD_LENGTH] = "/paramHM<0>/param1<6>[700]/param11<6>[1024]/../param12<6>[1038]/../../";
        if(strncmp((char*)obj->response, expected,PAYLOAD_LENGTH) == 0)
        {
            result = 1;
        }
        else
        {
            result = 0;
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
        }
    }
    ParamT_DeleteOneHandler(obj);
    return result;

}



uint8_t ParamT_TestGetTwoHandlers(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    ParamTestTwoHandlers_t* obj = ParamT_InitializeTwoHandlers(PAYLOAD_LENGTH);
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);
    }
    // Master receives the get message, and will first reply with its parameters in one or multiple messages,
    // here we know it will fit in one message.
    Event_Receive(obj->evHandlerMaster, 2);

    // Process the result.
    Event_Receive(obj->evHandlerTester, 2);

    // And validate that we got what we expected.
    uint8_t result = 1;

    {
        char expected[PAYLOAD_LENGTH] = "/paramHM<0>/param2<6>[0]/param21<6>[0]/../param22<6>[0]/../../";
        if(strncmp((char*)obj->response, expected,obj->responseSize) == 0)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(!(obj->lastInSequence))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Unexpectedly got lastInSequence!\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(obj->sequenceNr == 0)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected SequenceNr:0 \nGot:%d\n", obj->sequenceNr);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        obj->response[0] = '\0';
    }

    //we did not get lastInSequence, continue!
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);
    }
    // Master will receive the message, and pass it on to the other handler.
    Event_Receive(obj->evHandlerMaster, 2);

    // The master should have sent a get message to the other handler, process that message.
    Event_Receive(obj->evHandler, 2);

    // Now the other handler should have replied with its parameters, this reply goes to the
    // paramHandlerMaster, so lets poll that handler.

    Event_Receive(obj->evHandlerMaster, 2);

    // This should have generated a message to the caller, poll the tester.

    Event_Receive(obj->evHandlerTester, 2);

    // And validate that result.
    {
        char expected[PAYLOAD_LENGTH] = "/paramH<0>/param1<6>[0]/param11<6>[0]/../param12<6>[0]/../../";
        if((strncmp((char*)obj->response, expected,obj->responseSize) == 0))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if((obj->lastInSequence))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Did not get lastInSequence!\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(obj->sequenceNr == 1)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected SequenceNr:1 \nGot:%d\n", obj->sequenceNr);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
    }

    ParamT_DeleteTwoHandlers(obj);
    return result;

}





uint8_t ParamT_TestGetShortTwoHandlers(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    ParamTestTwoHandlers_t* obj = ParamT_InitializeTwoHandlers(PAYLOAD_LENGTH_SHORT);
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);
    }
    // Master receives the get message, and will first reply with its parameters in one or multiple messages,
    // here we know it will fit in one message.
    Event_Receive(obj->evHandlerMaster, 2);

    // Process the result.
    Event_Receive(obj->evHandlerTester, 2);

    // And validate that we got what we expected.
    uint8_t result = 1;
    {
        char expected[PAYLOAD_LENGTH] = "/paramHM<0>/param2<6>[0]/param21<6>[0]/../";
        if(strncmp((char*)obj->response, expected,obj->responseSize) == 0)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(!(obj->lastInSequence))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Unexpectedly got lastInSequence!\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(obj->sequenceNr == 0)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected SequenceNr:0 \nGot:%d\n", obj->sequenceNr);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        obj->response[0] = '\0';
    }

    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);
    }

    //We are only using one thread, so now we have to poll the receiver.
    Event_Receive(obj->evHandlerMaster, 2);

    //This should have generated a response, poll the sender.
    Event_Receive(obj->evHandlerTester, 2);

    {
        char expected[PAYLOAD_LENGTH] = "/paramHM<0>/param2<6>[0]/param22<6>[0]/../../";
        if(strncmp((char*)obj->response, expected,obj->responseSize) == 0)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(!(obj->lastInSequence))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Unexpectedly got lastInSequence!\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(obj->sequenceNr == 1)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected SequenceNr:1 \nGot:%d\n", obj->sequenceNr);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        obj->response[0] = '\0';
    }

    //we did not get lastInSequence, continue!
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);
    }
    // Master will receive the message, and pass it on to the other handler.
    Event_Receive(obj->evHandlerMaster, 2);

    // The master should have sent a get message to the other handler, process that message.
    Event_Receive(obj->evHandler, 2);

    // Now the other handler should have replied with its parameters, this reply goes to the
    // paramHandlerMaster, so lets poll that handler.

    Event_Receive(obj->evHandlerMaster, 2);

    // This should have generated a message to the caller, poll the tester.

    Event_Receive(obj->evHandlerTester, 2);

    // And validate that result.
    {
        char expected[PAYLOAD_LENGTH_SHORT] = "/paramH<0>/param1<6>[0]/param11<6>[0]/../";
        if((strncmp((char*)obj->response, expected,obj->responseSize) == 0))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(!(obj->lastInSequence))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Unexpectedly got lastInSequence!\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(obj->sequenceNr == 2)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected SequenceNr:2 \nGot:%d\n", obj->sequenceNr);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        obj->response[0] = '\0';
    }

    //we did not get lastInSequence, continue!
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);
    }
    // Master will receive the message, and pass it on to the other handler.
    Event_Receive(obj->evHandlerMaster, 2);

    // The master should have sent a get message to the other handler, process that message.
    Event_Receive(obj->evHandler, 2);

    // Now the other handler should have replied with its parameters, this reply goes to the
    // paramHandlerMaster, so lets poll that handler.

    Event_Receive(obj->evHandlerMaster, 2);

    // This should have generated a message to the caller, poll the tester.

    Event_Receive(obj->evHandlerTester, 2);

    // And validate that result.
    {
        char expected[PAYLOAD_LENGTH_SHORT] = "/paramH<0>/param1<6>[0]/param12<6>[0]/../../";
        if((strncmp((char*)obj->response, expected,obj->responseSize) == 0))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if((obj->lastInSequence))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Did not get lastInSequence!\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(obj->sequenceNr == 3)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected SequenceNr:3 \nGot:%d\n", obj->sequenceNr);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        obj->response[0] = '\0';
    }

    ParamT_DeleteTwoHandlers(obj);
    return result;

}




uint8_t ParamT_TestSetGetTwoHandlers(TestFw_t* Tobj)
{
    // initialize all objects used in the test.
    ParamTestTwoHandlers_t* obj = ParamT_InitializeTwoHandlers(PAYLOAD_LENGTH);


    {
        uint8_t payloadSet[PAYLOAD_LENGTH_LONG] = "/paramHM<0>/param2<6>[48]/param21<6>[63]/../param22<6>[88]/../../../paramH<0>/param1<6>[256]/param11<6>[512]/../param12<6>[1024]/\0";

        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_set, 0, 0, PAYLOAD_LENGTH);
        uint8_t* payload = Msg_ParamGetPayload(msg);

        payload = memcpy(payload, payloadSet, PAYLOAD_LENGTH);
        uint32_t payloadLength = strlen((char*)payload);

        Msg_ParamSetPayloadlength(msg, payloadLength);


        Event_Send(obj->evHandlerTester, msg);

        // Master receives the get message,
        Event_Receive(obj->evHandlerMaster, 2);

        // And send it to the slave
        Event_Receive(obj->evHandler, 2);

        // Master receives an ack.
        Event_Receive(obj->evHandlerMaster, 2);
    }


    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);

        // Master receives the get message, and will first reply with its parameters in one or multiple messages,
        // here we know it will fit in one message.
        Event_Receive(obj->evHandlerMaster, 2);

        // Process the result.
        Event_Receive(obj->evHandlerTester, 2);
    }
    // And validate that we got what we expected.
    uint8_t result = 1;

    {
        char expected[PAYLOAD_LENGTH] = "/paramHM<0>/param2<6>[48]/param21<6>[63]/../param22<6>[88]/../../";
        if(strncmp((char*)obj->response, expected,obj->responseSize) == 0)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s \nGot:\n%s\n", expected, obj->response);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(!(obj->lastInSequence))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Unexpectedly got lastInSequence!\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(obj->sequenceNr == 0)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected SequenceNr:0 \nGot:%d\n", obj->sequenceNr);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        obj->response[0] = '\0';
    }

    //we did not get lastInSequence, continue!
    {
        moduleMsg_t* msg = Msg_ParamCreate(FC_Param_e, 0, param_get, 0, 0, 0);
        Event_Send(obj->evHandlerTester, msg);
    }
    // Master will receive the message, and pass it on to the other handler.
    Event_Receive(obj->evHandlerMaster, 2);

    // The master should have sent a get message to the other handler, process that message.
    Event_Receive(obj->evHandler, 2);

    // Now the other handler should have replied with its parameters, this reply goes to the
    // paramHandlerMaster, so lets poll that handler.

    Event_Receive(obj->evHandlerMaster, 2);

    // This should have generated a message to the caller, poll the tester.

    Event_Receive(obj->evHandlerTester, 2);

    // And validate that result.
    {
        char expected[PAYLOAD_LENGTH] = "/paramH<0>/param1<6>[256]/param11<6>[512]/../param12<6>[1024]/../../\0";
        if(obj->receivedSize > obj->responseSize)
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"received size to large.");
            TestFW_Report(Tobj, tmpstr);
            result = 0; 
        }
        if(result && (strncmp((char*)obj->response, expected,obj->receivedSize) == 0))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected:\n%s\nWith %d bytes, \nGot:\n%s\nWith %ld bytes", expected, strlen(expected), obj->response, obj->receivedSize);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if((obj->lastInSequence))
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Did not get lastInSequence!\n");
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
        if(obj->sequenceNr == 1)
        {
            result &= 1;
        }
        else
        {
            char tmpstr[REPORT_LONG] = {0};
            snprintf (tmpstr, REPORT_LONG,"Expected SequenceNr:1 \nGot:%d\n", obj->sequenceNr);
            TestFW_Report(Tobj, tmpstr);
            result = 0;
        }
    }
    ParamT_DeleteTwoHandlers(obj);
    return result;

}

uint8_t ParamT_HandleGetParamOneHandler(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    ParamTestOneHandler_t* testObj = (ParamTestOneHandler_t*)data;

    memcpy(testObj->response, Msg_ParamGetPayload(msg),testObj->responseSize);
    testObj->done = Msg_ParamGetLastinsequence(msg) ? 1 : 0;
    return 1;
}

uint8_t ParamT_HandleGetParam(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    ParamTestTwoHandlers_t* testObj = (ParamTestTwoHandlers_t*)data;

    testObj->receivedSize = Msg_ParamGetPayloadlength(msg);
    memcpy(testObj->response, Msg_ParamGetPayload(msg), testObj->responseSize);
    testObj->done = Msg_ParamGetLastinsequence(msg) ? 1 : 0;
    testObj->lastInSequence = Msg_ParamGetLastinsequence(msg);
    testObj->sequenceNr = Msg_ParamGetSequencenr(msg);
    return 1;
}

