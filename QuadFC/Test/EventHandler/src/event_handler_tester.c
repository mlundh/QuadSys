/*
 * event_handler_tester.c
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


/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include <stdio.h>
#include <string.h>
#include "Test/EventHandler/event_handler_tester.h"
#include "EventHandler/inc/event_handler.h"
#include "EventHandler/inc/event_circular_buffer.h"

#define EV_HANDLER_INTENRNAL_IDX (0)


/**
 * Struct only used here to pass parameters to the tasks.
 * The struct contains the tasks event handler, and a queue
 * to which the task can respond to the test fw.
 */
typedef struct testEventTaskParams
{
    eventHandler_t* evHandler;
    QueueHandle_t   responseQueue;
    TaskHandle_t xHandle;
}testEventTaskParams;

typedef struct eventTester
{
    QueueHandle_t responseQueue;
    testEventTaskParams* taskParam1;
    testEventTaskParams* taskParam2;
    testEventTaskParams* taskParam3;
    testEventTaskParams* stimuliTask;
} eventTester_t;




/**
 * Response type for the testcases.
 */
typedef struct testResponse
{
    eventHandler_t* evHandler;
    messageTypes_t  event;
}testResponse_t;

/**
 * Create the tasks used to span the event mesh.
 */
uint8_t EventHandler_createTestTasks(eventTester_t* obj);
void EventHandler_deleteTestTasks(eventTester_t* obj);

void event_test_task( void *pvParameters );
void stimuli_test_task( void *pvParameters );

uint8_t EventHandler_TestCreateDelete(TestFw_t* obj);
uint8_t EventHandler_TestCreateDeleteCbuff(TestFw_t* obj);


/**
 * Callback functions for the event handlers in this test case. The
 * functions will send a message to the response queue of the main test
 * task (where testFW is running).
 */
uint8_t testMsg_GetUiCommands_eCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t testTestEvent2Cb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t testMsg_FcFault_eCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t stimuliPeripheralErrorCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t stateCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t ctrlCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);
uint8_t setpointCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data);



void EventHandler_GetTCs(TestFw_t* obj)
{
    TestFW_RegisterTest(obj, "Event CreateDelete", EventHandler_TestCreateDelete);
    TestFW_RegisterTest(obj, "Event CreateDeleteCbuff", EventHandler_TestCreateDeleteCbuff);
    TestFW_RegisterTest(obj, "Event CBR/W", EventHandler_TestCBReadWrite);
    TestFW_RegisterTest(obj, "Event BCFullEmpty", EventHandler_TestCBFullEmpty);
    TestFW_RegisterTest(obj, "Event", EventHandler_TestEvents);
}



#define SHORT_MSG (50)

uint8_t EventHandler_TestCreateDelete(TestFw_t* obj)
{
        eventHandler_t* evHandlerM = Event_CreateHandler(GS_SerialIO_e, 1);
        eventHandler_t* evHandlerS = Event_CreateHandler(GS_Param_e, 0);


        Event_DeleteHandler(evHandlerM);
        Event_DeleteHandler(evHandlerS);
        return 1;
}

uint8_t EventHandler_TestCreateDeleteCbuff(TestFw_t* obj)
{
    eventCircBuffer_t* eventBuffer = Event_CreateCBuff();
    eventCircBuffer_t* eventBuffer1 = Event_CreateCBuff();


        Event_DeleteCBuff(eventBuffer);
        Event_DeleteCBuff(eventBuffer1);
        return 1;
}



uint8_t EventHandler_TestCBReadWrite(TestFw_t* obj)
{
    uint8_t result = 1;
    // Create the event buffer.
    eventCircBuffer_t* eventBuffer = Event_CreateCBuff();

    // Populate the first two positions. Data is copied, scoped here in test case
    // to ensure nothing strange is going on.
    {
        moduleMsg_t* event1Stimuli = Msg_Create(Msg_CtrlMode_e, 0);
        Event_CBuffPushBack(eventBuffer, event1Stimuli);

        moduleMsg_t* event2Stimuli = Msg_Create(Msg_CtrlSig_e, 0);
        Event_CBuffPushBack(eventBuffer, event2Stimuli);
    }

    // Read back the events. We are only queueing pointers..
    moduleMsg_t* event1Result;
    Event_CBuffPopFront(eventBuffer, &event1Result);

    moduleMsg_t* event2Result;
    Event_CBuffPopFront(eventBuffer, &event2Result);

    if(!(event1Result->type == Msg_CtrlMode_e) ||
            !(event2Result->type == Msg_CtrlSig_e))
    {
        result = 0; // did not get the expected result.
    }
    vPortFree(eventBuffer);
    Msg_Delete(&event1Result);
    Msg_Delete(&event2Result);

    return result;
}

uint8_t EventHandler_TestCBFullEmpty(TestFw_t* obj)
{

    uint8_t result = 1;
    // Create the event buffer.
    eventCircBuffer_t* eventBuffer = Event_CreateCBuff();
    if(!eventBuffer)
    {
        TestFW_Report(obj, "ERROR: Not able to create circular buffer.\n");
        return 0;
    }
    // Now try to write to all locations in the buffer.
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        moduleMsg_t* event = Msg_Create(Msg_CtrlSig_e, 0);

        if(1 != Event_CBuffPushBack(eventBuffer, event))
        {
            TestFW_Report(obj, "ERROR: Not able to write data to buffer.\n");
            result = 0; // We were not able to write enough data to the buffer.
        }
    }

    // Try to write another entry, this should fail.
    moduleMsg_t* event = Msg_Create(Msg_CtrlModeReq_e, 0);

    if(0 != Event_CBuffPushBack(eventBuffer, event))
    {
        TestFW_Report(obj, "ERROR: Write successful when buffer was full.\n");
        result = 0; // We were able to write to the full buffer, not good!
    }
    Msg_Delete(&event);


    // Read all positions in the buffer.
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        moduleMsg_t* event1;
        if(1 != Event_CBuffPopFront(eventBuffer, &event1))
        {
            TestFW_Report(obj, "ERROR: Not able to read data from buffer.\n");
            result = 0; // We were not able to read enough data from the buffer.
        }
        if(event1->type != Msg_CtrlSig_e)
        {
            TestFW_Report(obj, "ERROR: Did not get the right event.\n");
            result = 0; // Expected a control signal event.
        }
        Msg_Delete(&event1);
    }

    // Try to read another entry, this should fail.
    moduleMsg_t* event1;
    if(0 != Event_CBuffPopFront(eventBuffer, &event1))
    {
        TestFW_Report(obj, "ERROR: Read successful when buffer was empty.\n");
        result = 0; // We were able to read from the empty buffer, not good!
    }

    // Populate the first two positions again, we have wrapped around.

    moduleMsg_t* event1Stimuli = Msg_Create(Msg_CtrlModeReq_e, 0);
    Event_CBuffPushBack(eventBuffer, event1Stimuli);

    moduleMsg_t* event2Stimuli = Msg_Create(Msg_CtrlSig_e, 0);
    Event_CBuffPushBack(eventBuffer, event2Stimuli);

    // Read back the events.
    moduleMsg_t* event1Result;
    Event_CBuffPopFront(eventBuffer, &event1Result);

    moduleMsg_t* event2Result;
    Event_CBuffPopFront(eventBuffer, &event2Result);

    if(!(event1Result->type == Msg_CtrlModeReq_e) ||
            !(event2Result->type == Msg_CtrlSig_e))
    {
        TestFW_Report(obj, "ERROR: Did not get the correct events back from buffer.\n");
        result = 0; // did not get the expected result.
    }
    Msg_Delete(&event1Result);
    Msg_Delete(&event2Result);
    Event_DeleteCBuff(eventBuffer);

    return result;
}

#define REPORTLEN (100)
uint8_t EventHandler_TestEvents(TestFw_t* obj)
{

    eventTester_t* eventTester = pvPortMalloc(sizeof(eventTester_t));
    eventTester->responseQueue = xQueueCreate( 6,(sizeof(testResponse_t)));

    TestFW_Report(obj, "- Creating test tasks.\n");
    // Creation of tasks will start the testcase.
    EventHandler_createTestTasks(eventTester);

    // The expected responses.
    uint8_t nr_exp_rsp = 4;
    testResponse_t expectedRsp[nr_exp_rsp];
    // From handler 1
    expectedRsp[0].evHandler = eventTester->taskParam1->evHandler;
    expectedRsp[0].event = Msg_NewSetpoint_e;
    expectedRsp[1].evHandler = eventTester->taskParam1->evHandler;
    expectedRsp[1].event = Msg_CtrlSig_e;
    // From handler 2
    expectedRsp[2].evHandler = eventTester->taskParam2->evHandler;
    expectedRsp[2].event = Msg_NewSetpoint_e;
    // From handler 3
    expectedRsp[3].evHandler = eventTester->taskParam3->evHandler;
    expectedRsp[3].event = Msg_NewSetpoint_e;

    TestFW_Report(obj, "- Reading test results of first part of test.\n");
    testResponse_t rsp[nr_exp_rsp];
    uint8_t result = 1;
    for(int i = 0; i < nr_exp_rsp; i++)
    {
        if(xQueueReceive(eventTester->responseQueue, &rsp[i], 1000/portTICK_PERIOD_MS) == pdTRUE)
        {
            // Check received message against all expected messages. If one matches then
            // the event notification succeed for that event.

            uint8_t found = 0;
            int j = 0;
            for( j = 0; j < nr_exp_rsp; j++)
            {
                if((expectedRsp[j].evHandler == rsp[i].evHandler) &&
                        (expectedRsp[j].event     == rsp[i].event    ))
                {
                    // Event response found.
                    found++;
                    break;
                }
            }
            if(found != 1)
            {
                TestFW_Report(obj, "---Expected events was not found.\n");
                char tmpstr[REPORTLEN] = {0};
                snprintf(tmpstr, REPORTLEN, "Expected: Handler: %p, event %d\nGot:      Handler: %p, event %d\n",expectedRsp[j].evHandler,expectedRsp[j].event,rsp[i].evHandler,rsp[i].event);
                TestFW_Report(obj, tmpstr);
                result = 0;
            }
        }
        else
        {
            TestFW_Report(obj, "---No events in queue.\n");
            return 0; // error receiving the event, timeout or other.
        }
    }

    TestFW_Report(obj, "- Reading test results of second part of test.\n");

    // Second part of the test case, buffering.
    testResponse_t testRsp;
    messageTypes_t expected[] = {Msg_FireUiCommand_e,Msg_FcFault_e};
    for(int i = 0; i < 2; i++)
    {
        if(xQueueReceive(eventTester->responseQueue, &testRsp, 10000/portTICK_PERIOD_MS) == pdTRUE)
        {
            if(testRsp.event != expected[i])
            {
                TestFW_Report(obj, "---Expected events was not found.\n");
                char tmpstr[REPORTLEN] = {0};
                snprintf(tmpstr, REPORTLEN, "Expected: %d\nGot: %d\n", expected[i], testRsp.event);
                TestFW_Report(obj, tmpstr);
                result = 0;
            }
        }
        else
        {
            TestFW_Report(obj, "---No events in queue.\n");
            return 0; // error receiving the event, timeout or other.
        }
    }

    EventHandler_deleteTestTasks(eventTester);

    vTaskDelay(5);//Give the idle task time to clean up..
    return result;
}


uint8_t EventHandler_createTestTasks(eventTester_t* obj)
{
    // Create all three tasks. They will be stimulated from the main task.


    testEventTaskParams * taskParam1 = pvPortMalloc(sizeof(testEventTaskParams));
    taskParam1->evHandler = Event_CreateHandler(FC_Dbg_e, 1);
    // Task 1 is interested in new state and new control signal.
    taskParam1->responseQueue = obj->responseQueue;
    obj->taskParam1 = taskParam1;
    Event_RegisterCallback(taskParam1->evHandler, Msg_CtrlSig_e ,ctrlCb, taskParam1);
    Event_RegisterCallback(taskParam1->evHandler, Msg_NewSetpoint_e ,stateCb, taskParam1);



    testEventTaskParams * taskParam2 = pvPortMalloc(sizeof(testEventTaskParams));
    taskParam2->evHandler = Event_CreateHandler(FC_Log_e, 0);
    // Task 2 is interested in new state.
    taskParam2->responseQueue = obj->responseQueue;
    obj->taskParam2 = taskParam2;

    Event_RegisterCallback(taskParam2->evHandler, Msg_NewSetpoint_e ,stateCb, taskParam2);



    testEventTaskParams * taskParam3 = pvPortMalloc(sizeof(testEventTaskParams));
    taskParam3->evHandler = Event_CreateHandler(FC_SerialIOrx_e, 0);
    // Task 3 is interested in new setpoint.
    taskParam3->responseQueue = obj->responseQueue;
    obj->taskParam3 = taskParam3;

    Event_RegisterCallback(taskParam3->evHandler, Msg_NewSetpoint_e ,setpointCb, taskParam3);

    Event_RegisterCallback(taskParam3->evHandler, Msg_GetUiCommands_e ,testMsg_GetUiCommands_eCb, taskParam3);
    Event_RegisterCallback(taskParam3->evHandler, Msg_FireUiCommand_e ,testTestEvent2Cb, taskParam3);
    Event_RegisterCallback(taskParam3->evHandler, Msg_FcFault_e ,testMsg_FcFault_eCb, taskParam3);

    testEventTaskParams * stimuliTask = pvPortMalloc(sizeof(testEventTaskParams));
    stimuliTask->evHandler = Event_CreateHandler(FC_Param_e, 0);
    // stimuliTask is not interested in any event.
    stimuliTask->responseQueue = obj->responseQueue;
    obj->stimuliTask = stimuliTask;

    Event_RegisterCallback(stimuliTask->evHandler, Msg_Error_e ,stimuliPeripheralErrorCb, stimuliTask);


    Event_InitHandler(taskParam1->evHandler, taskParam2->evHandler);
    Event_InitHandler(taskParam1->evHandler, taskParam3->evHandler);
    Event_InitHandler(taskParam1->evHandler, stimuliTask->evHandler);

    portBASE_TYPE create_result1 = xTaskCreate( event_test_task,   /* The function that implements the task.  */
            (const char *const) "Test1",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
            300,                                          /* The stack size for the task*/
            taskParam1,                                   /* pass params to task.*/
            configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
            &taskParam1->xHandle );                                       /* Handle to the task. Not used here and therefore NULL*/


    portBASE_TYPE create_result2 = xTaskCreate( event_test_task,   /* The function that implements the task.  */
            (const char *const) "Test2",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
            300,                                          /* The stack size for the task*/
            taskParam2,                                   /* pass params to task.*/
            configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
            &taskParam2->xHandle );                                       /* Handle to the task. Not used here and therefore NULL*/



    portBASE_TYPE create_result3 = xTaskCreate( event_test_task,   /* The function that implements the task.  */
            (const char *const) "Test3",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
            300,                                          /* The stack size for the task*/
            taskParam3,                                   /* pass params to task.*/
            configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
            &taskParam3->xHandle );                                       /* Handle to the task. Not used here and therefore NULL*/


    portBASE_TYPE create_result4 = xTaskCreate( stimuli_test_task,   /* The function that implements the task.  */
            (const char *const) "Stimuli",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
            300,                                          /* The stack size for the task*/
            stimuliTask,                                   /* pass params to task.*/
            configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
            &stimuliTask->xHandle );                                       /* Handle to the task. Not used here and therefore NULL*/

    if ( create_result1 != pdTRUE || create_result2 != pdTRUE || create_result3 != pdTRUE || create_result4 != pdTRUE)
    {
        return 0;
    }



    return 1;
}

void EventHandler_deleteTestTasks(eventTester_t* obj)
{
    Event_DeleteHandler(obj->taskParam1->evHandler);
    vPortFree(obj->taskParam1);
    Event_DeleteHandler(obj->taskParam2->evHandler);
    vPortFree(obj->taskParam2);
    Event_DeleteHandler(obj->taskParam3->evHandler);
    vPortFree(obj->taskParam3);
    Event_DeleteHandler(obj->stimuliTask->evHandler);
    vPortFree(obj->stimuliTask);
    vQueueDelete(obj->responseQueue);
    vTaskDelete(obj->taskParam1->xHandle);
    vTaskDelete(obj->taskParam2->xHandle);
    vTaskDelete(obj->taskParam3->xHandle);
    vTaskDelete(obj->stimuliTask->xHandle);
    vPortFree(obj);

}


void stimuli_test_task( void *pvParameters )
{
    testEventTaskParams * param = (testEventTaskParams*)(pvParameters);


    // WARNING! This is not the proper way of creating messages, use the appropriate create functions instead. This is just for testing.
    {
        moduleMsg_t* msg = Msg_Create(Msg_NewSetpoint_e, FC_Broadcast_e);
        Event_Send(param->evHandler, msg);
    }
    {
        moduleMsg_t* msg = Msg_Create(Msg_CtrlSig_e, FC_Broadcast_e);
        Event_Send(param->evHandler, msg);
    }
    {
     //   moduleMsg_t* msg = Msg_Create(Msg_NewSetpoint_e, Broadcast);
     //   Event_Send(param->evHandler, msg);
    }
    // All tasks will send Msg_Error_e event to the stimuli task
    // when the first part of the test is finished. If they did not then
    // there would be no guarantee that the events would be sent to the
    // test framework task in the correct order.
    //
    for (int i = 0; i < 4; i++ )
    {
        Event_Receive(param->evHandler, portMAX_DELAY);
    }


    {
        moduleMsg_t* msg = Msg_Create(Msg_GetUiCommands_e, FC_Broadcast_e);
        Event_Send(param->evHandler, msg);
    }
    {
        moduleMsg_t* msg = Msg_Create(Msg_FcFault_e, FC_Broadcast_e);
        Event_Send(param->evHandler, msg);
    }
    {
        moduleMsg_t* msg = Msg_Create(Msg_FireUiCommand_e, FC_Broadcast_e);
        Event_Send(param->evHandler, msg);
    }

    for ( ;; )
    {
        Event_Receive(param->evHandler, portMAX_DELAY);
    }

}

void event_test_task( void *pvParameters )
{
    testEventTaskParams * param = (testEventTaskParams*)(pvParameters);


    for ( ;; )
    {
        Event_Receive(param->evHandler, portMAX_DELAY);
    }

}

// Task 3 will use this callback.
uint8_t testMsg_GetUiCommands_eCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
    Event_WaitForEvent(obj, Msg_FireUiCommand_e, 1, 1, portMAX_DELAY);
    Event_HandleBufferedEvents(obj);
    return 1;
}
uint8_t testTestEvent2Cb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
    testEventTaskParams * param = (testEventTaskParams*)(taskParam);
    testResponse_t eventRsp;
    eventRsp.evHandler = obj;
    eventRsp.event = data->type;
    xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
    return 1;
}
uint8_t testMsg_FcFault_eCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
    testEventTaskParams * param = (testEventTaskParams*)(taskParam);
    testResponse_t eventRsp;
    eventRsp.evHandler = obj;
    eventRsp.event = data->type;
    xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
    return 1;
}
uint8_t stimuliPeripheralErrorCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
    // Only used for synchronization purposes.
    return 1;
}

uint8_t stateCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
    testEventTaskParams * param = (testEventTaskParams*)(taskParam);
    testResponse_t eventRsp;
    eventRsp.evHandler = obj;
    eventRsp.event = Msg_NewSetpoint_e;
    xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
    // Sync with stimuli thread
    moduleMsg_t* msg = Msg_Create(Msg_Error_e, FC_Broadcast_e);
    Event_Send(param->evHandler, msg);
    return 1;
}
uint8_t ctrlCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
    testEventTaskParams * param = (testEventTaskParams*)(taskParam);
    testResponse_t eventRsp;
    eventRsp.evHandler = obj;
    eventRsp.event = Msg_CtrlSig_e;
    xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
    // Sync with stimuli thread
    moduleMsg_t* msg = Msg_Create(Msg_Error_e, FC_Broadcast_e);
    Event_Send(param->evHandler, msg);
    return 1;
}
uint8_t setpointCb(eventHandler_t* obj, void* taskParam, moduleMsg_t* data)
{
    testEventTaskParams * param = (testEventTaskParams*)(taskParam);
    testResponse_t eventRsp;
    eventRsp.evHandler = obj;
    eventRsp.event = Msg_NewSetpoint_e;
    xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
    // Sync with stimuli thread
    moduleMsg_t* msg = Msg_Create(Msg_Error_e, FC_Broadcast_e);
    Event_Send(param->evHandler, msg);
    return 1;
}
