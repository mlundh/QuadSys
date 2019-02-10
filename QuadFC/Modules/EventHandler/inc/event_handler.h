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
#include "MsgBase/inc/common_types.h"
#include "MsgBase/inc/message_base.h"
#include "MsgBase/inc/msgAddr.h"
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




typedef struct eventHandler eventHandler_t;

#define NR_QUEUES_MAX (8)
#define EVENT_QUEUE_LENGTH (16)
#define EVENT_QUEUE_ITEM_SIZE ((sizeof(moduleMsg_t*)))

#define EVENT_BLOCK_TIME (0)

/**
 * Typedef for handler functions for events.
 * @param data    Event data. Set to null if no data is expected.
 * @return        1 if success, 0 otherwise.
 */
typedef uint8_t (*eventHandlerFcn)(eventHandler_t* obj, void* data, moduleMsg_t* eData);


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
	msgAddr_t         handlerId;
	uint32_t          subscriptions;
	eventHandler_t*   handlers[NR_QUEUES_MAX];                    //Registered handlers. Only use queue after the scheduler has been started.
    msgAddr_t         handlerIds[NR_QUEUES_MAX];
	uint32_t          handlerSubscriptions[NR_QUEUES_MAX];        // Bit access for each handler.
	eventHandlerFcn   eventFcns[Msg_LastType_e];                 // Registered event handlers.
	void*             eventDataBinding[Msg_LastType_e];          // Registered data to be used for specific events.
	uint8_t           registeredHandlers;
	uint8_t           master;
	eventCircBuffer_t* cBuffer;
};

/**
 * Use this function to create a new handler. The function can be
 * called before the scheduler is started.
 * @return
 */
eventHandler_t* Event_CreateHandler(msgAddr_t id, uint8_t master);

/**
 * Delete and event handler.
 */
void Event_DeleteHandler(eventHandler_t* );

/**
 * Register the event handler to the master. This function has to be called
 * before the scheduler is started.
 * @param master  Master handler. Only one is allowed in the system.
 *
 * @param obj     Current handler.
 * @return
 */
uint8_t Event_InitHandler(eventHandler_t* master, eventHandler_t* obj);

/**
 * Get the ID of the current handler.
 * @param obj
 * @return
 */
msgAddr_t Event_GetId(eventHandler_t* obj);
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
uint8_t Event_Subscribe(eventHandler_t* obj, messageTypes_t event);

/**
 * Unsubscribe to an event type. Events of this type will not be
 * delivered into the current handlers event mailbox.
 * @param obj     Current handler.
 * @param event   Event to unsubscribe from.
 * @return        1 if successful, 0 otherwise.
 */
uint8_t Event_Unsubscribe(eventHandler_t* obj, messageTypes_t event);

/**
 * Send event nr event to all event handlers that has registered for
 * that particular event.
 * @param obj     Current handler.
 * @param event   Event to be sent.
 * @return        1 if successful, 0 otherwise.
 */
uint8_t Event_Send(eventHandler_t* obj, moduleMsg_t* event);

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
uint8_t Event_WaitForEvent(eventHandler_t* obj, messageTypes_t event, uint8_t waitForNr, uint8_t bufferEvents);

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
 * result in an error. Must be called before the scheduler is started.
 * @param obj           Current handler.
 * @param eventNumber   Event number that should trigger the callback.
 * @param fcn           Callback function.
 * @param data          Call the callback function with the registered data instead of task param data.
 * @return
 */
uint8_t Event_RegisterCallback(eventHandler_t* obj, messageTypes_t eventNumber, eventHandlerFcn fcn, void* data);




#endif /* MODULES_EVENTHANDLER_INC_EVENT_HANDLER_H_ */
