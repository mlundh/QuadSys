/*
* rc_sp_handler.c
*
* Copyright (C) 2022  Martin Lundh
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
#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"
#include "Messages/inc/Msg_NewRcSetpoint.h"
#include "GenericRcSetpointHandler/inc/rc_sp_handler.h"
#include "EventHandler/inc/event_handler.h"
#include "Messages/inc/Msg_ValidSp.h"

/**
 * Interface function for sending setpoints.If a still valid, higher setpoint
 * is already registered then the new setpoint will be ignored.
 * @param obj           Current setpoint object.
 * @param setpoint      Setpoint to send.
 * @param valid_for_ms  Time until when the setpoint should be considered too old.
 * @param prio          Priority of setpoint.
 * @return          1 if successful, 0 otherwise.
 */
uint8_t RcSpHandl_SetSetpoint(RcSpHandler_t *obj, genericRC_t* setpoint, uint8_t valid_for_ms, uint8_t prio );


//timer callback is handled in the service task, and not in the task owning the setpoint handler.
void RcSpHandl_TimerCallback( TimerHandle_t pxTimer );

struct RcSetpointData
{
    genericRC_t setpoint;
    uint8_t prio;
};
typedef struct RcSetpointData RcSetpointData_t;

struct RcSpObj
{
    QueueHandle_t xQueue_Sp_Handl;
    TimerHandle_t xTimer;
    uint32_t      validSp;
    eventHandler_t* evHandler;
};

const static uint8_t timerBlockTimeMs = 1;

/*Setpoint queue*/
#define SP_HANDL_QUEUE_LENGTH      (1)
#define SP_HANDL_QUEUE_ITEM_SIZE         (sizeof(RcSetpointData_t))

uint8_t RcSpHandl_SpCB(eventHandler_t* obj, void* data, moduleMsg_t* msg);


RcSpHandler_t *RcSpHandl_Create(eventHandler_t* eHandler)
{
    RcSpHandler_t* obj = pvPortMalloc(sizeof(RcSpHandler_t));
    obj->xQueue_Sp_Handl = xQueueCreate( SP_HANDL_QUEUE_LENGTH, SP_HANDL_QUEUE_ITEM_SIZE );
    obj->xTimer = xTimerCreate("Timer", 1, pdFALSE, ( void * ) obj, RcSpHandl_TimerCallback);
    obj->validSp = 0;
    if(!obj || ! obj->xQueue_Sp_Handl)
    {
        return NULL;
    }
    Event_RegisterCallback(eHandler, Msg_NewRcSetpoint_e, RcSpHandl_SpCB, obj);
    obj->evHandler = eHandler;
    return obj;
}


uint8_t RcSpHandl_SetSetpoint(RcSpHandler_t* obj, genericRC_t* setpoint, uint8_t valid_for_ms, uint8_t prio )
{
    uint8_t result = 1;
    RcSetpointData_t setpointdata = {
            .setpoint = *setpoint, // Shallow copy is all that is needed!
            .prio = prio};

    RcSetpointData_t inQueue = {{{0}}};
    //if there is a message in the queue check if it has a higher prio then the current item.
    if(uxQueueMessagesWaiting(obj->xQueue_Sp_Handl))
    {
        if (xQueuePeek(obj->xQueue_Sp_Handl, &inQueue, timerBlockTimeMs/ portTICK_PERIOD_MS) != pdPASS )
        {
            return 0;
        }
        else
        {
            // Only let a setpoint with a equal or higher priority overwrite an active setpoint.
            if(inQueue.prio > setpointdata.prio)
            {
                return 2; // Valid to be refused to submit setpoint if the active setpoint has a higher priority.
            }
        }
    }

    //Update the period of the timer.
    if( xTimerChangePeriod( obj->xTimer, valid_for_ms / portTICK_PERIOD_MS, timerBlockTimeMs / portTICK_PERIOD_MS ) != pdPASS )
    {
        //Clear the queue, we are not able to set a correct timer value!
        xQueueReset(obj->xQueue_Sp_Handl);
        obj->validSp = 0;
        return 0;
    }
    // Reset the timer.
    if( xTimerReset( obj->xTimer, timerBlockTimeMs / portTICK_PERIOD_MS ) != pdPASS )
    {
        //Clear the queue, we are not able to set a correct timer value!
        xQueueReset(obj->xQueue_Sp_Handl);
        obj->validSp = 0;
        return 0;
    }
    BaseType_t write_result = xQueueOverwrite( obj->xQueue_Sp_Handl, &setpointdata );
    if(write_result != pdTRUE)
    {
        result = 0; // Should never happen.
    }
    return result;
}

uint8_t RcSpHandl_GetSetpoint(RcSpHandler_t *obj, genericRC_t *setpoint, uint8_t timeout_ms )
{
    if(!uxQueueMessagesWaiting(obj->xQueue_Sp_Handl))
    {
        return 0;
    }
    // Peek the queue, the timer callback will remove the item from the queue when it expires.
    if (xQueuePeek(obj->xQueue_Sp_Handl, setpoint, timeout_ms/ portTICK_PERIOD_MS) == pdPASS )
    {
        return 1;
    }
    return 0;
}

void RcSpHandl_TimerCallback( TimerHandle_t pxTimer )
{
    RcSpHandler_t *obj = ( RcSpHandler_t * ) pvTimerGetTimerID( pxTimer );
    xQueueReset(obj->xQueue_Sp_Handl);
    obj->validSp = 0;
    moduleMsg_t* msg = Msg_ValidSpCreate(FC_Broadcast_e, 0, obj->validSp);
    Event_Send(obj->evHandler, msg);
    //TODO write error message!
}


uint8_t RcSpHandl_SpCB(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || !data || !msg)
    {
        return 0;
    }
    RcSpHandler_t* setpointHobj = (RcSpHandler_t*)data; // data should always be the current handler.
    genericRC_t state = Msg_NewRcSetpointGetSetpoint(msg); 

    RcSpHandl_SetSetpoint(setpointHobj, &state, Msg_NewRcSetpointGetValidfor(msg), Msg_NewRcSetpointGetPriority(msg));
    if(!setpointHobj->validSp)
    {
        setpointHobj->validSp = 1;
        moduleMsg_t* msgSp = Msg_ValidSpCreate(FC_Broadcast_e, 0, setpointHobj->validSp);
        Event_Send(obj, msgSp);
    }
    return 1;
}

