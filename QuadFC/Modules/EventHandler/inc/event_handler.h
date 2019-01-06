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
#include "Modules/EventHandler/inc/event_circular_buffer.h"

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
 * the master event handler. The handler then needs to be initialized
 * by a call to Event_InitHandler(), that will return when all
 * handlers in the system are initialized.
 *
 *
 * eInitialize and eInitializeDone has default handlers that do nothing.
 * The main purpose of these events is to synchronize between tasks
 * using Event_WaitForEvent(). They are also default subscribed.
 */




#define NR_QUEUES_MAX (8)
#define EVENT_QUEUE_LENGTH (16)
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

  eNewCtrlMode,    //!< eNewCtrlMode     New flight control mode.
  eNewFlightMode,  //!< eNewFlightMode   New flight mode (initializing, unarmed, armed etc.).

  eInitialize,     //!< eInitialize      Start of initialization for tasks.
  eInitializeDone, //!< eInitializeDone  Initialization is done.

  eNewSetpoint,    //!< eNewSetpoint     New setpoint has arrived.
  eNewCtrlSignal,  //!< eNewCtrlSignal   New control signal has been calculated.
  eNewState,       //!< eNewState        New state vector has been calculated.

  eFcFault,        //!< eFcFault         FC fault. Serious error condition.
  ePeripheralError,//!< ePeripheralError Not able to interact with a peripheral.

  eTestEvent1,     //!< eTestEvent1      Test event 1.
  eTestEvent2,     //!< eTestEvent2      Test event 2.

  eLog,            //!< eLog             A log event.
  eLogNameReq,     //!< eLogNameReq      A requirement for the log name-id mapping
  eLogStop,        //!< eLogStop         A request to stop all logging

  eNrEvents        //!< eNrEvents
}event_t;

/**
 * Typedef for handler functions for events.
 * @param data    Event data. Set to null if no data is expected.
 * @return        1 if success, 0 otherwise.
 */
typedef uint8_t (*eventHandlerFcn)(eventHandler_t* obj, void* data, eventData_t* eData);

/**
 * Struct describing an event. This is the structure that will be
 * sent over the internal queue. The data field should be used with caution,
 * nothing in the event system guarantees that it is thread safe. So use for sending
 * structures that are them self thread safe, this could be a new queue that contains
 * the actual data.
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
 * the struct is sent on the event queue(this is done in the initialization
 * phase, when setting up the mesh, and when changing subscriptions),
 * and given when the receiving task is done with the data. This is
 * the signal to the owner that the data is free to use again.
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
  void*             eventDataBinding[eNrEvents];          // Registered data to be used for specific events.
  uint8_t           nrHandlers;
  uint8_t           registeredHandlers;
  uint8_t           meshComplete;
  eventCircBuffer_t* cBuffer;
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
eventHandler_t* Event_CreateHandler(QueueHandle_t masterQueue, uint8_t nr_handlers);

/**
 * Initialize the event handler and the task it belongs to.
 * This might take some time since there will be multiple messages sent on the queues
 * spanning the event mesh.
 *
 * This function has to be called after the scheduler is started.
 *
 * If not followed by Event_EndInitialize the thread will hang. The reason
 * for this behavior is to let the task initialize any resources before any
 * part of the system is started.
 * @param obj     Current handler.
 * @return
 */
void Event_StartInitialize(eventHandler_t* obj);

/**
 * End the initialization of the task.
 * @param obj
 */
void Event_EndInitialize(eventHandler_t* obj);


/**
 * Subscribe to an event type. Events of this type will be put into
 * the current handlers event mailbox.
 * @param obj     Current handler.
 * @param event   Event to subscribe to.
 * @return        1 if successful, 0 otherwise.
 */
uint8_t Event_Subscribe(eventHandler_t* obj, event_t event);

/**
 * Unsubscribe to an event type. Events of this type will not be
 * delivered into the current handlers event mailbox.
 * @param obj     Current handler.
 * @param event   Event to unsubscribe from.
 * @return        1 if successful, 0 otherwise.
 */
uint8_t Event_Unsubscribe(eventHandler_t* obj, event_t event);

/**
 * Send event nr event to all event handlers that has registered for
 * that particular event.
 * @param obj     Current handler.
 * @param event   Event to be sent.
 * @return        1 if successful, 0 otherwise.
 */
uint8_t Event_Send(eventHandler_t* obj, eventData_t event);

/**
 * Process events from the mailbox. The events are processed as they arrive. If there
 * are no events to handle then the function will return 0 after blockTime.
 * @param obj           Current handler.
 * @param blockTime     Time the task can block. 0 for no block.
 * @return              1 if the task processed an event, 0 if timeout.
 */
uint8_t Event_Receive(eventHandler_t* obj, uint32_t blockTime);

/**
 * Wait for the event specified by the event parameter. The function can either
 * queue up to BUFFER_SIZE number of events, or process them as they come. It will only
 * return once the event it is waiting for has arrived.
 * @param obj           Current handler.
 * @param event         Event to wait for.
 * @param waitForNr     Set to number of events of the type to wait for. If set
 *                      to 0 the call will return without doing anything.
 * @param bufferEvents  set to 1 to buffer events, and 0 if the events should be
 * processed as they arrive.
 * @return              1 if the event waited for was properly handled, 0 if an error occured.
 */
uint8_t Event_WaitForEvent(eventHandler_t* obj, event_t event, uint8_t waitForNr, uint8_t bufferEvents);

/**
 * Handle events buffered by Event_WaitForEvent if buffering was enabled.
 * @param obj           Current handler.
 * @return              1 if an event was successfully handled, 0 otherwise.
 */
uint8_t Event_HandleBufferedEvents(eventHandler_t* obj);

/**
 * Register a callback function for a specific event. Only events
 * with callbacks will be processed by this handler. Subscribing to
 * an event that does not have a handler in the current handler is
 * an error. Every received event that does not have a handler will
 * result in an error.
 * @param obj           Current handler.
 * @param eventNumber   Event number that should trigger the callback.
 * @param fcn           Callback function.
 * @param data          Call the callback function with the registered data instead of task param data.
 * @return
 */
uint8_t Event_RegisterCallback(eventHandler_t* obj, event_t eventNumber, eventHandlerFcn fcn, void* data);




#endif /* MODULES_EVENTHANDLER_INC_EVENT_HANDLER_H_ */
