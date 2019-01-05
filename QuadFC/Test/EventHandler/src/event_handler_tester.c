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

typedef struct eventTester
{
  QueueHandle_t responseQueue;
  eventHandler_t* evHandler1; // Used as id.
  eventHandler_t* evHandler2;
  eventHandler_t* evHandler3;
  eventHandler_t* evHandler4;
} eventTester_t;


/**
 * Struct only used here to pass parameters to the tasks.
 * The struct contains the tasks event handler, and a queue
 * to which the task can respond to the test fw.
 */
typedef struct testEventTaskParams
{
  eventHandler_t* evHandler;
  QueueHandle_t   responseQueue;
}testEventTaskParams;

/**
 * Response type for the testcases.
 */
typedef struct testResponse
{
  eventHandler_t* evHandler;
  event_t         event;
}testResponse_t;

/**
 * Create the tasks used to span the event mesh.
 */
uint8_t EventHandler_createTestTasks(eventTester_t* obj);
void event_test_task( void *pvParameters );
void stimuli_test_task( void *pvParameters );

/**
 * Callback functions for the event handlers in this test case. The
 * functions will send a message to the response queue of the main test
 * task (where testFW is running).
 */
uint8_t testeTestEvent1Cb(eventHandler_t* obj, void* taskParam, eventData_t* data);
uint8_t testTestEvent2Cb(eventHandler_t* obj, void* taskParam, eventData_t* data);
uint8_t testeFcFaultCb(eventHandler_t* obj, void* taskParam, eventData_t* data);
uint8_t stimuliPeripheralErrorCb(eventHandler_t* obj, void* taskParam, eventData_t* data);
uint8_t stateCb(eventHandler_t* obj, void* taskParam, eventData_t* data);
uint8_t ctrlCb(eventHandler_t* obj, void* taskParam, eventData_t* data);
uint8_t setpointCb(eventHandler_t* obj, void* taskParam, eventData_t* data);



void EventHandler_GetTCs(TestFw_t* obj)
{
  TestFW_RegisterTest(obj, "CBR/W", EventHandler_TestCBReadWrite);
  TestFW_RegisterTest(obj, "BCFullEmpty", EventHandler_TestCBFullEmpty);
  TestFW_RegisterTest(obj, "Event", EventHandler_TestEvents);

  eventTester_t* eventTester = pvPortMalloc(sizeof(eventTester_t));
  eventTester->responseQueue = xQueueCreate( 6,(sizeof(testResponse_t)));
  TestFW_SetTestSuiteInternal(obj, (void*)eventTester, EV_HANDLER_INTENRNAL_IDX);
}




uint8_t EventHandler_TestCBReadWrite(TestFw_t* obj)
{
  uint8_t result = 1;
  // Create the event buffer.
  eventCircBuffer_t* eventBuffer = Event_CreateCBuff();

  // Populate the first two positions. Data is copied, scoped here in test case
  // to ensure nothing strange is going on.
  {
    eventData_t event1Stimuli;
    event1Stimuli.data=NULL;
    event1Stimuli.eventNumber = eNewCtrlMode;
    Event_CBuffPushBack(eventBuffer, &event1Stimuli);

    eventData_t event2Stimuli;
    event2Stimuli.data=NULL;
    event2Stimuli.eventNumber = eNewCtrlSignal;
    Event_CBuffPushBack(eventBuffer, &event2Stimuli);
  }

  // Read back the events.
  eventData_t event1Result;
  Event_CBuffPopFront(eventBuffer, &event1Result);

  eventData_t event2Result;
  Event_CBuffPopFront(eventBuffer, &event2Result);

  if(!(event1Result.eventNumber == eNewCtrlMode) ||
      !(event2Result.eventNumber == eNewCtrlSignal))
  {
    result = 0; // did not get the expected result.
  }



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
    eventData_t event;
    event.eventNumber = eNewCtrlSignal;
    if(1 != Event_CBuffPushBack(eventBuffer, &event))
    {
      TestFW_Report(obj, "ERROR: Not able to write data to buffer.\n");
      result = 0; // We were not able to write enough data to the buffer.
    }
  }

  // Try to write another entry, this should fail.
  eventData_t event;
  event.eventNumber = eNewCtrlMode;
  if(0 != Event_CBuffPushBack(eventBuffer, &event))
  {
    TestFW_Report(obj, "ERROR: Write successful when buffer was full.\n");
    result = 0; // We were able to write to the full buffer, not good!
  }

  // Read all positions in the buffer.
  for(int i = 0; i < BUFFER_SIZE; i++)
  {
    eventData_t event1;
    if(1 != Event_CBuffPopFront(eventBuffer, &event1))
    {
      TestFW_Report(obj, "ERROR: Not able to read data from buffer.\n");
      result = 0; // We were not able to read enough data from the buffer.
    }
    if(event1.eventNumber != eNewCtrlSignal)
    {
      TestFW_Report(obj, "ERROR: Did not get the right event.\n");
      result = 0; // Expected a control signal event.
    }
  }

  // Try to read another entry, this should fail.
  eventData_t event1;
  if(0 != Event_CBuffPopFront(eventBuffer, &event1))
  {
    TestFW_Report(obj, "ERROR: Read successful when buffer was empty.\n");
    result = 0; // We were able to read from the empty buffer, not good!
  }

  // Populate the first two positions again, we have wrapped around.

  eventData_t event1Stimuli;
  event1Stimuli.data=NULL;
  event1Stimuli.eventNumber = eNewCtrlMode;
  Event_CBuffPushBack(eventBuffer, &event1Stimuli);

  eventData_t event2Stimuli;
  event2Stimuli.data=NULL;
  event2Stimuli.eventNumber = eNewCtrlSignal;
  Event_CBuffPushBack(eventBuffer, &event2Stimuli);

  // Read back the events.
  eventData_t event1Result;
  Event_CBuffPopFront(eventBuffer, &event1Result);

  eventData_t event2Result;
  Event_CBuffPopFront(eventBuffer, &event2Result);

  if(!(event1Result.eventNumber == eNewCtrlMode) ||
      !(event2Result.eventNumber == eNewCtrlSignal))
  {
    TestFW_Report(obj, "ERROR: Did not get the correct events back from buffer.\n");
    result = 0; // did not get the expected result.
  }
  return result;
}
#define REPORTLEN (100)
uint8_t EventHandler_TestEvents(TestFw_t* obj)
{
  eventTester_t* eventTester =  (eventTester_t*)TestFW_GetTestSuiteInternal(obj, EV_HANDLER_INTENRNAL_IDX);
  TestFW_Report(obj, "- Creating test tasks.\n");
  // Creation of tasks will start the testcase.
  EventHandler_createTestTasks(eventTester);

  // The expected responses.
  uint8_t nr_exp_rsp = 4;
  testResponse_t expectedRsp[nr_exp_rsp];
  // From handler 1
  expectedRsp[0].evHandler = eventTester->evHandler1;
  expectedRsp[0].event = eNewState;
  expectedRsp[1].evHandler = eventTester->evHandler1;
  expectedRsp[1].event = eNewCtrlSignal;
  // From handler 2
  expectedRsp[2].evHandler = eventTester->evHandler2;
  expectedRsp[2].event = eNewState;
  // From handler 3
  expectedRsp[3].evHandler = eventTester->evHandler3;
  expectedRsp[3].event = eNewSetpoint;

  TestFW_Report(obj, "- Reading test results of first part of test.\n");
  testResponse_t rsp[nr_exp_rsp];
  uint8_t result = 1;
  for(int i = 0; i < nr_exp_rsp; i++)
  {
    if(xQueueReceive(eventTester->responseQueue, &rsp[i], 1000/portTICK_PERIOD_MS) == pdTRUE)
    {
      {
        char tmpstr[REPORTLEN] = {0};
        snprintf(tmpstr, REPORTLEN, "Received: Handler: %p, event %d\n",rsp[i].evHandler,rsp[i].event);
        TestFW_Report(obj, tmpstr);
      }
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
  event_t expected[] = {eTestEvent2,eFcFault};
  for(int i = 0; i < 2; i++)
  {
    if(xQueueReceive(eventTester->responseQueue, &testRsp, 10000/portTICK_PERIOD_MS) == pdTRUE)
    {
      if(testRsp.event != expected[i])
      {
        TestFW_Report(obj, "---Expected events was not found.\n");
        result = 0;
      }
    }
    else
    {
      TestFW_Report(obj, "---No events in queue.\n");
      return 0; // error receiving the event, timeout or other.
    }
  }

  return result;
}


uint8_t EventHandler_createTestTasks(eventTester_t* obj)
{
  // Create all three tasks. They will be stimulated from the main task.


  testEventTaskParams * taskParam1 = pvPortMalloc(sizeof(testEventTaskParams));
  taskParam1->evHandler = Event_CreateHandler(NULL, 4);
  // Task 1 is interested in new state and new control signal.
  taskParam1->responseQueue = obj->responseQueue;
  obj->evHandler1 = taskParam1->evHandler;
  Event_RegisterCallback(taskParam1->evHandler, eNewCtrlSignal ,ctrlCb, taskParam1);
  Event_RegisterCallback(taskParam1->evHandler, eNewState ,stateCb, taskParam1);
  portBASE_TYPE create_result1 = xTaskCreate( event_test_task,   /* The function that implements the task.  */
      (const char *const) "Test1",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
      100,                                          /* The stack size for the task*/
      taskParam1,                                   /* pass params to task.*/
      configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

  testEventTaskParams * taskParam2 = pvPortMalloc(sizeof(testEventTaskParams));
  taskParam2->evHandler = Event_CreateHandler(taskParam1->evHandler->eventQueue, 0);
  // Task 2 is interested in new state.
  taskParam2->responseQueue = obj->responseQueue;
  obj->evHandler2 = taskParam2->evHandler;

  Event_RegisterCallback(taskParam2->evHandler, eNewState ,stateCb, taskParam2);
  portBASE_TYPE create_result2 = xTaskCreate( event_test_task,   /* The function that implements the task.  */
      (const char *const) "Test2",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
      100,                                          /* The stack size for the task*/
      taskParam2,                                   /* pass params to task.*/
      configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

  testEventTaskParams * taskParam3 = pvPortMalloc(sizeof(testEventTaskParams));
  taskParam3->evHandler = Event_CreateHandler(taskParam1->evHandler->eventQueue, 0);
  // Task 3 is interested in new setpoint.
  taskParam3->responseQueue = obj->responseQueue;
  obj->evHandler3 = taskParam3->evHandler;
  Event_RegisterCallback(taskParam3->evHandler, eNewSetpoint ,setpointCb, taskParam3);

  Event_RegisterCallback(taskParam3->evHandler, eTestEvent1 ,testeTestEvent1Cb, taskParam3);
  Event_RegisterCallback(taskParam3->evHandler, eTestEvent2 ,testTestEvent2Cb, taskParam3);
  Event_RegisterCallback(taskParam3->evHandler, eFcFault ,testeFcFaultCb, taskParam3);

  portBASE_TYPE create_result3 = xTaskCreate( event_test_task,   /* The function that implements the task.  */
      (const char *const) "Test3",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
      100,                                          /* The stack size for the task*/
      taskParam3,                                   /* pass params to task.*/
      configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

  testEventTaskParams * stimuliTask = pvPortMalloc(sizeof(testEventTaskParams));
  stimuliTask->evHandler = Event_CreateHandler(taskParam1->evHandler->eventQueue, 0);
  // stimuliTask is not interested in any event.
  stimuliTask->responseQueue = obj->responseQueue;
  obj->evHandler4 = stimuliTask->evHandler;
  Event_RegisterCallback(stimuliTask->evHandler, ePeripheralError ,stimuliPeripheralErrorCb, stimuliTask);
  portBASE_TYPE create_result4 = xTaskCreate( stimuli_test_task,   /* The function that implements the task.  */
      (const char *const) "Stimuli",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
      100,                                          /* The stack size for the task*/
      stimuliTask,                                   /* pass params to task.*/
      configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

  if ( create_result1 != pdTRUE || create_result2 != pdTRUE || create_result3 != pdTRUE || create_result4 != pdTRUE)
  {
    return 0;
  }

  return 1;
}

void stimuli_test_task( void *pvParameters )
{
  testEventTaskParams * param = (testEventTaskParams*)(pvParameters);
  Event_StartInitialize(param->evHandler);
  Event_EndInitialize(param->evHandler);


  eventData_t evData = {0};
  evData.eventNumber = eNewSetpoint;
  Event_Send(param->evHandler, evData);

  evData.eventNumber = eNewCtrlSignal;
  Event_Send(param->evHandler, evData);

  evData.eventNumber = eNewState;
  Event_Send(param->evHandler, evData);

  // All tasks will send ePeripheralError event to the stimuli task
  // when the first part of the test is finished. If they did not then
  // there would be no guarantee that the events would be sent to the
  // test framework task in the correct order.
  //
  for (int i = 0; i < 4; i++ )
  {
    Event_Receive(param->evHandler, portMAX_DELAY);
  }

  // Second part of test, testing the buffering of events.
  evData.eventNumber = eTestEvent1;
  Event_Send(param->evHandler, evData);

  // This is not the event the task is waiting for.
  evData.eventNumber = eFcFault;
  Event_Send(param->evHandler, evData);

  // Now send the event the task is waiting for.
  evData.eventNumber = eTestEvent2;
  Event_Send(param->evHandler, evData);

  for ( ;; )
  {
    Event_Receive(param->evHandler, portMAX_DELAY);
  }

}

void event_test_task( void *pvParameters )
{
  testEventTaskParams * param = (testEventTaskParams*)(pvParameters);
  Event_StartInitialize(param->evHandler);
  Event_EndInitialize(param->evHandler);


  for ( ;; )
  {
    Event_Receive(param->evHandler, portMAX_DELAY);
  }

}

// Task 3 will use this callback.
uint8_t testeTestEvent1Cb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  Event_WaitForEvent(obj, eTestEvent2, 1, 1);
  Event_HandleBufferedEvents(obj);
  return 1;
}
uint8_t testTestEvent2Cb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  testEventTaskParams * param = (testEventTaskParams*)(taskParam);
  testResponse_t eventRsp;
  eventRsp.evHandler = obj;
  eventRsp.event = data->eventNumber;
  xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
  return 1;
}
uint8_t testeFcFaultCb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  testEventTaskParams * param = (testEventTaskParams*)(taskParam);
  testResponse_t eventRsp;
  eventRsp.evHandler = obj;
  eventRsp.event = data->eventNumber;
  xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
  return 1;
}
uint8_t stimuliPeripheralErrorCb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  // Only used for synchronization purposes.
  return 1;
}

uint8_t stateCb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  testEventTaskParams * param = (testEventTaskParams*)(taskParam);
  testResponse_t eventRsp;
  eventRsp.evHandler = obj;
  eventRsp.event = eNewState;
  xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
  // Sync with stimuli thread
  eventData_t evData = {0};
  evData.eventNumber = ePeripheralError;
  Event_Send(param->evHandler, evData);
  return 1;
}
uint8_t ctrlCb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  testEventTaskParams * param = (testEventTaskParams*)(taskParam);
  testResponse_t eventRsp;
  eventRsp.evHandler = obj;
  eventRsp.event = eNewCtrlSignal;
  xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
  // Sync with stimuli thread
  eventData_t evData = {0};
  evData.eventNumber = ePeripheralError;
  Event_Send(param->evHandler, evData);
  return 1;
}
uint8_t setpointCb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  testEventTaskParams * param = (testEventTaskParams*)(taskParam);
  testResponse_t eventRsp;
  eventRsp.evHandler = obj;
  eventRsp.event = eNewSetpoint;
  xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
  // Sync with stimuli thread
  eventData_t evData = {0};
  evData.eventNumber = ePeripheralError;
  Event_Send(param->evHandler, evData);
  return 1;
}
