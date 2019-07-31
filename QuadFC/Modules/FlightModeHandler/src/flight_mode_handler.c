/*
 * state_handler.c
 *
 * Copyright (C) 2015 Martin Lundh
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
#include "FlightModeHandler/inc/flight_mode_handler.h"
#include "MsgBase/inc/msgAddr.h"

#include "EventHandler/inc/event_handler.h"

struct FlightModeHandler
{
    FlightMode_t current_mode;
    eventHandler_t* eHandler;
};


static BaseType_t FMode_ChangeAllowed(FlightModeHandler_t* obj, FlightMode_t state_req);

void FMode_SendEvent(FlightModeHandler_t*  obj);

uint8_t FMode_CB(eventHandler_t* obj, void* data, moduleMsg_t* eData);


FlightModeHandler_t* FMode_CreateFmodeHandler(eventHandler_t* eHandler)
{
    FlightModeHandler_t* obj = pvPortMalloc(sizeof(FlightModeHandler_t));

    obj->current_mode = fmode_not_available;
    obj->eHandler = eHandler;
    Event_RegisterCallback(obj->eHandler, Msg_FlightModeReq_e, FMode_CB, obj);
    return obj;
}


/*Initialize queues and mutex used internally
 * by the state handler*/
void FMode_InitFModeHandler(FlightModeHandler_t*  obj)
{
    obj->current_mode = fmode_init;
    FMode_SendEvent(obj);
}

FlightMode_t FMode_GetCurrent(FlightModeHandler_t* obj)
{
    return obj->current_mode;
}



BaseType_t FMode_ChangeFMode(FlightModeHandler_t*  obj, FlightMode_t state_req)
{
    BaseType_t result = pdFALSE;
    if(FMode_ChangeAllowed(obj, state_req) == pdTRUE)
    {
        obj->current_mode = state_req;
        FMode_SendEvent(obj);
        result = pdTRUE;
    }
    return result;
}

BaseType_t FMode_Fault(FlightModeHandler_t* obj)
{
    obj->current_mode = fmode_fault;
    FMode_SendEvent(obj);
    return pdTRUE;
}


static BaseType_t FMode_ChangeAllowed(FlightModeHandler_t* obj, FlightMode_t state_req)
{
    switch ( FMode_GetCurrent(obj) )
    {
    case fmode_init:
        if(    (state_req == fmode_disarming) )
        {
            return pdTRUE;
        }
        break;
    case fmode_disarmed:
        if(    (state_req == fmode_config)
                || (state_req == fmode_init )
                || (state_req == fmode_armed )
                || (state_req == fmode_arming )
                || (state_req == fmode_fault ) )
        {
            return pdTRUE;
        }
        break;
    case fmode_config:
        if(    (state_req == fmode_disarming)
                || (state_req == fmode_fault ) )
        {
            return pdTRUE;
        }
        break;
    case fmode_arming:
        if(    (state_req == fmode_armed)
                || (state_req == fmode_disarming )
                || (state_req == fmode_fault ) )
        {
            return pdTRUE;
        }
        break;
    case fmode_armed:
        if(    (state_req == fmode_disarming)
                || (state_req == fmode_fault ) )
        {
            return pdTRUE;
        }
        break;
    case   fmode_disarming:
        if(    (state_req == fmode_disarmed)
                || (state_req == fmode_fault ) )
        {
            return pdTRUE;
        }
        break;
    case fmode_fault:
        if(    (state_req == fmode_disarming) )
        {
            return pdTRUE;
        }
        break;
    default:
        return pdFALSE;
        break;
    }

    return pdFALSE;
}

uint8_t FMode_CB(eventHandler_t* obj, void* data, moduleMsg_t* eData)
{
    if(!obj || !data || !eData)
    {
        return 0;
    }
    FlightModeHandler_t* FMobj = (FlightModeHandler_t*)data; // data should always be the current handler.
    uint8_t result = FMode_ChangeFMode(FMobj, Msg_FlightModeReqGetMode(eData));
    if(!result)
    {
        moduleMsg_t* msg = Msg_FlightModeCreate(eData->mSource, 0, FMobj->current_mode);
        Event_Send(FMobj->eHandler, msg);
    }
    return 1;
}

void FMode_SendEvent(FlightModeHandler_t*  obj)
{
    if(obj && obj->eHandler)
    {
        moduleMsg_t* msg = Msg_FlightModeCreate(Broadcast, 0,obj->current_mode);
        Event_Send(obj->eHandler, msg);
    }
}


