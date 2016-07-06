/*
 * event_handler.h
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
#ifndef MODULES_EVENTHANDLER_INC_EVENT_HANDLER_H_
#define MODULES_EVENTHANDLER_INC_EVENT_HANDLER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "Utilities/inc/common_types.h"

/**
 * @file event_handler.h
 * Provides an interface for the event_handler. All tasks will be
 * able to signal events through the event handler. Each event
 * will be sent to the tasks subscribing to that particular event
 * type.
 *
 * Every event handler is a part of the event handler mesh. This
 * mesh is set up in the initialization of the event handlers. It
 * is necessary that all event handlers are created at task startup,
 * and that the total number of handlers in the system is given to
 * the master event handler.
 */




#define NR_QUEUES_MAX (8)
#define EVENT_QUEUE_LENGTH (8)
#define EVENT_QUEUE_ITEM_SIZE ((sizeof(eventData_t)))

#define EVENT_BLOCK_TIME (0)
/**
 * All possible events are here. Add new events before eNrEvents.
 */
typedef enum events
{
  eRegisterHandler,//!< eRegisterHandler Register a new handler.
  eMeshComplete,   //!< eMeshComplete    Mesh is complete, it is safe to use the mesh data structure.
  eSubscribe,      //!< eSubscribe       New subscription.
  eUnsubscribe,    //!< eUnsubscribe     New unsubscription.
  eNewSetpoint,    //!< eNewSetpoint     New setpoint has arrived.
  eNewCtrlSignal,  //!< eNewCtrlSignal   New control signal has been calculated.
  eNewState,       //!< eNewState        New state vector has been calculated.
  eInitialize,     //!< eInitialize      Start of initialization for tasks.
  eInitializeDone, //!< eInitializeDone  Initialization is done.
  eNrEvents        //!< eNrEvents
}event_t;

/**
 * Typedef for handler functions for events.
 * @param data    Event data. Set to null if no data is expected.
 * @return        1 if success, 0 otherwise.
 */
typedef uint8_t (*eventHandlerFcn)(eventHandler_t* obj, void* taskParam, eventData_t* data);

/**
 * Struct describing an event. This is the structure that will be
 * sent over the internal queue.
 */
struct eventData
{
  event_t         eventNumber;
  void*           data;
};
/**
 * Event handler struct, contains all information each handler needs.
 * A pointer to this struct is sent when the mesh is setup at event system
 * initialization time.
 *
 * eventHandler data is used internally by the event_handler. The
 * data here is protected by a semaphore, it should be empty when
 * the struct is sent on the event queue, and given when the
 * receiving task is done with the data. This is the signal to the
 * owner that the data is free.
 */
struct eventHandler
{
  QueueHandle_t     eventQueue;
  uint8_t           handlerId;
  uint32_t          subscriptions;
  SemaphoreHandle_t semaphore;
  QueueHandle_t     handlers[NR_QUEUES_MAX];              //Registered handler queues.
  uint32_t          handlerSubscriptions[NR_QUEUES_MAX];  // Bit access for each handler.
  eventHandlerFcn   eventFcns[eNrEvents];                 // Registered event handlers.
  uint8_t           nrHandlers;
  uint8_t           registeredHandlers;
  uint8_t           meshComplete;
};

/**
 * Use this function to create a new handler. The function can be
 * called before the scheduler is started.
 * @param masterQueue       NULL if this is the the master event handler,
 *                          otherwise the master event handler queue.
 * @param nr_handlers       Number of event handlers in the system. Only
 *                          used by the master handler.
 * @return
 */
eventHandler_t* createEventHandler(QueueHandle_t* masterQueue, uint8_t nr_handlers);

/**
 * Initialize the event handler. This might take some time
 * since there will be multiple messages sent on the queues
 * spanning the event mesh. This function has to be called after the
 * scheduler is started.
 * @param obj     Current handler.
 * @return
 */
uint8_t initEventHandler(eventHandler_t* obj);

/**
 * Subscribe to an event type. Events of this type will be put into
 * the current handlers event mailbox.
 * @param obj     Current handler.
 * @param events  Event to subscribe to.
 * @return        1 if successful, 0 otherwise.
 */
uint8_t subscribeEvents(eventHandler_t* obj, uint32_t events);

/**
 * Unsubscribe to an event type. Events of this type will not be
 * delivered into the current handlers event mailbox.
 * @param obj     Current handler.
 * @param events  Event to unsubscribe from.
 * @return        1 if successful, 0 otherwise.
 */
uint8_t unsubscribeEvents(eventHandler_t* obj, uint32_t events);

/**
 * Send event nr event to all event handlers that has registered for
 * that particular event.
 * @param obj     Current handler.
 * @param event   Event to be sent.
 * @return        1 if successful, 0 otherwise.
 */
uint8_t sendEvent(eventHandler_t* obj, eventData_t event);

/**
 * Process events from the mailbox.
 * @param obj           Current handler.
 * @param taskParam     Task parameters, could store internal task state.
 * @param blockTime     Time the task can block. 0 for no block.
 * @return
 */
uint8_t receiveEvent(eventHandler_t* obj, void* taskParam, uint32_t blockTime);

/**
 * Register a callback function for a specific event. Only events
 * with callbacks will be processed by this handler. Subscribing to
 * an event that does not have a handler in the current handler is
 * an error. Every received event that does not have a handler will
 * result in an error.
 * @param obj           Current handler.
 * @param eventNumber   Event number that should trigger the callback.
 * @param fcn           Callback function.
 * @return
 */
uint8_t registerEventCallback(eventHandler_t* obj, event_t eventNumber, eventHandlerFcn fcn);

#endif /* MODULES_EVENTHANDLER_INC_EVENT_HANDLER_H_ */
