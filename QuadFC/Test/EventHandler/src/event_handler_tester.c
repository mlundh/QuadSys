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
uint8_t stateCb(eventHandler_t* obj, void* taskParam, eventData_t* data);
uint8_t ctrlCb(eventHandler_t* obj, void* taskParam, eventData_t* data);
uint8_t setpointCb(eventHandler_t* obj, void* taskParam, eventData_t* data);



void EventHandler_GetTCs(TestFw_t* obj)
{
  TestFW_RegisterTest(obj, "Event", EventHandler_TestEvents);

  eventTester_t* eventTester = pvPortMalloc(sizeof(eventTester_t));
  eventTester->responseQueue = xQueueCreate( 6,(sizeof(testResponse_t)));
  TestFW_SetTestSuiteInternal(obj, (void*)eventTester, EV_HANDLER_INTENRNAL_IDX);



}

uint8_t EventHandler_TestEvents(TestFw_t* obj)
{

  char tmpstr[100] = {0};
  eventTester_t* eventTester =  (eventTester_t*)TestFW_GetTestSuiteInternal(obj, EV_HANDLER_INTENRNAL_IDX);
  snprintf (tmpstr, 100,"- Creating test tasks.\n");
  TestFW_Report(obj, tmpstr);
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

  snprintf (tmpstr, 100,"- Reading test results.\n");
  TestFW_Report(obj, tmpstr);
  testResponse_t rsp[nr_exp_rsp];
  uint8_t result = 1;
  for(int i = 0; i < nr_exp_rsp; i++)
  {
    if(xQueueReceive(eventTester->responseQueue, &rsp[i], 100/portTICK_PERIOD_MS) == pdTRUE)
    {
      // Check received message against all expected messages. If one matches then
      // the event notification succeed for that event.
      uint8_t found = 0;
      for(int j = 0; j < nr_exp_rsp; j++)
      {
        if((expectedRsp[j].evHandler == rsp[i].evHandler) &&
           (expectedRsp[j].event     == rsp[i].event    ))
        {
          // Event response found.
          found++;
        }
      }
      if(found != 1)
      {
        snprintf (tmpstr, 100,"Expected events was not found.\n");
        TestFW_Report(obj, tmpstr);
        result = 0;
      }
    }
    else
    {
      return 0; // error receiving the event, timeout or other.
    }
  }
  return result;
}

uint8_t EventHandler_createTestTasks(eventTester_t* obj)
{
  // Create all three tasks. They will be stimulated from the main task.


  testEventTaskParams * taskParam1 = pvPortMalloc(sizeof(testEventTaskParams));
  taskParam1->evHandler = createEventHandler(NULL, 4);
  // Task 1 is interested in new state and new control signal.
  taskParam1->evHandler->subscriptions = (1 << eNewState) | (1 << eNewCtrlSignal);
  taskParam1->responseQueue = obj->responseQueue;
  obj->evHandler1 = taskParam1->evHandler;
  registerEventCallback(taskParam1->evHandler, eNewState ,stateCb);
  registerEventCallback(taskParam1->evHandler, eNewCtrlSignal ,ctrlCb);
  registerEventCallback(taskParam1->evHandler, eNewSetpoint ,setpointCb);
  portBASE_TYPE create_result1 = xTaskCreate( event_test_task,   /* The function that implements the task.  */
      (const char *const) "Test1",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
      100,                                          /* The stack size for the task*/
      taskParam1,                                   /* pass params to task.*/
      configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

  testEventTaskParams * taskParam2 = pvPortMalloc(sizeof(testEventTaskParams));
  taskParam2->evHandler = createEventHandler(taskParam1->evHandler->eventQueue, 0);
  // Task 2 is interested in new state.
  taskParam2->evHandler->subscriptions = (1 << eNewState);
  taskParam2->responseQueue = obj->responseQueue;
  obj->evHandler2 = taskParam2->evHandler;
  registerEventCallback(taskParam2->evHandler, eNewState ,stateCb);
  registerEventCallback(taskParam2->evHandler, eNewCtrlSignal ,ctrlCb);
  registerEventCallback(taskParam2->evHandler, eNewSetpoint ,setpointCb);
  portBASE_TYPE create_result2 = xTaskCreate( event_test_task,   /* The function that implements the task.  */
      (const char *const) "Test2",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
      100,                                          /* The stack size for the task*/
      taskParam2,                                   /* pass params to task.*/
      configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

  testEventTaskParams * taskParam3 = pvPortMalloc(sizeof(testEventTaskParams));
  taskParam3->evHandler = createEventHandler(taskParam1->evHandler->eventQueue, 0);
  // Task 3 is interested in new setpoint.
  taskParam3->evHandler->subscriptions = (1 << eNewSetpoint);
  taskParam3->responseQueue = obj->responseQueue;
  obj->evHandler3 = taskParam3->evHandler;
  registerEventCallback(taskParam3->evHandler, eNewState ,stateCb);
  registerEventCallback(taskParam3->evHandler, eNewCtrlSignal ,ctrlCb);
  registerEventCallback(taskParam3->evHandler, eNewSetpoint ,setpointCb);
  portBASE_TYPE create_result3 = xTaskCreate( event_test_task,   /* The function that implements the task.  */
      (const char *const) "Test3",                  /* The name of the task. This is not used by the kernel, only aids in debugging*/
      100,                                          /* The stack size for the task*/
      taskParam3,                                   /* pass params to task.*/
      configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
      NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

  testEventTaskParams * stimuliTask = pvPortMalloc(sizeof(testEventTaskParams));
  stimuliTask->evHandler = createEventHandler(taskParam1->evHandler->eventQueue, 0);
  // stimuliTask is not interested in any event.
  stimuliTask->evHandler->subscriptions = 0;
  stimuliTask->responseQueue = obj->responseQueue;
  obj->evHandler4 = stimuliTask->evHandler;
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
  if(initEventHandler(param->evHandler) != 1)
  {
    for(;;)
    {
      // ERROR!
    }
  }
  eventData_t evData = {0};
  evData.eventNumber = eNewSetpoint;
  sendEvent(param->evHandler, evData);

  evData.eventNumber = eNewCtrlSignal;
  sendEvent(param->evHandler, evData);

  evData.eventNumber = eNewState;
  sendEvent(param->evHandler, evData);

  for ( ;; )
  {
    receiveEvent(param->evHandler, param, portMAX_DELAY);
  }

}

void event_test_task( void *pvParameters )
{
  testEventTaskParams * param = (testEventTaskParams*)(pvParameters);
  if(initEventHandler(param->evHandler) != 1)
  {
    for(;;)
    {
      // ERROR!
    }
  }

  for ( ;; )
  {
    receiveEvent(param->evHandler, param, portMAX_DELAY);
  }

}

uint8_t stateCb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  testEventTaskParams * param = (testEventTaskParams*)(taskParam);
  testResponse_t eventRsp;
  eventRsp.evHandler = obj;
  eventRsp.event = eNewState;
  xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
  return 1;
}
uint8_t ctrlCb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  testEventTaskParams * param = (testEventTaskParams*)(taskParam);
  testResponse_t eventRsp;
  eventRsp.evHandler = obj;
  eventRsp.event = eNewCtrlSignal;
  xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
  return 1;
}
uint8_t setpointCb(eventHandler_t* obj, void* taskParam, eventData_t* data)
{
  testEventTaskParams * param = (testEventTaskParams*)(taskParam);
  testResponse_t eventRsp;
  eventRsp.evHandler = obj;
  eventRsp.event = eNewSetpoint;
  xQueueSend(param->responseQueue, &eventRsp, EVENT_BLOCK_TIME);
  return 1;
}
