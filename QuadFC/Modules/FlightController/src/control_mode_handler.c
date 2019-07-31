/*
 * control_mode_handler.c
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
#include "../inc/control_mode_handler.h"
#include "Messages/inc/Msg_CtrlMode.h"
#include "Messages/inc/Msg_CtrlModeReq.h"


struct CtrlModeHandler
{
    CtrlMode_t currentMode;
    eventHandler_t* eHandler;
};


/**
 * Queue holding current mode. Implemented
 * as a queue to ensure thread safety.
 */
#define MODE_CURRENT_QUEUE_LENGTH      (1)
#define MODE_CURRENT_QUEUE_ITEM_SIZE        (sizeof(CtrlMode_t))


static uint8_t Ctrl_ModeChangeAllowed(CtrlModeHandler_t* obj, CtrlMode_t mode_req);

uint8_t Ctrl_ModeCB(eventHandler_t* obj, void* data, moduleMsg_t* eData);

void Ctrl_SendEvent(CtrlModeHandler_t*  obj);

CtrlModeHandler_t* Ctrl_CreateModeHandler(eventHandler_t* eHandler)
{
    CtrlModeHandler_t* obj = pvPortMalloc(sizeof(CtrlModeHandler_t));
    if(!obj)
    {
        return NULL;
    }
    obj->currentMode = Control_mode_not_available;
    obj->eHandler = eHandler;
    Event_RegisterCallback(obj->eHandler, Msg_CtrlModeReq_e, Ctrl_ModeCB, obj);

    return obj;
}


void Ctrl_InitModeHandler(CtrlModeHandler_t* obj)
{

    obj->currentMode = Control_mode_rate;
    Ctrl_SendEvent(obj);

}

/**
 * Get the current mode.
 */
CtrlMode_t Ctrl_GetCurrentMode(CtrlModeHandler_t* obj)
{
    return obj->currentMode;
}



uint8_t Ctrl_ChangeMode(CtrlModeHandler_t* obj, CtrlMode_t mode_req)
{
    uint8_t result = 0;
    if(Ctrl_ModeChangeAllowed(obj, mode_req) == pdTRUE)
    {
        obj->currentMode = mode_req;
        Ctrl_SendEvent(obj);
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

static uint8_t Ctrl_ModeChangeAllowed(CtrlModeHandler_t* obj, CtrlMode_t mode_req)
{
    switch ( Ctrl_GetCurrentMode(obj) )
    {
    case Control_mode_rate:
        if(    (mode_req == Control_mode_attitude) )
        {
            return 1;
        }
        break;
    case Control_mode_attitude:
        if(    (mode_req == Control_mode_rate))
        {
            return 1;
        }
        break;
    default:
        return 1;
        break;
    }

    return 0;
}

uint8_t Ctrl_ModeCB(eventHandler_t* obj, void* data, moduleMsg_t* eData)
{
    if(!obj || !data || !eData)
    {
        return 0;
    }
    CtrlModeHandler_t* Mobj = (CtrlModeHandler_t*)data; // data should always be the current handler.
    uint8_t result = Ctrl_ChangeMode(Mobj, Msg_CtrlModeReqGetMode(eData));
    if(!result)
    {
        moduleMsg_t* msg = Msg_CtrlModeCreate(eData->mSource, 0, Mobj->currentMode);
        Event_Send(Mobj->eHandler, msg);
    }
    return 1;
}


void Ctrl_SendEvent(CtrlModeHandler_t*  obj)
{
    if(obj->eHandler && obj)
    {
        moduleMsg_t* msg = Msg_CtrlModeCreate(Broadcast, 0,obj->currentMode);
        Event_Send(obj->eHandler, msg);
    }
}


