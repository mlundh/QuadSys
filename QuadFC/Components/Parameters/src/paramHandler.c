/*
 * paramHandler.c
 *
 * Copyright (C) 2019 Martin Lundh
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


#include "../inc/paramHandler.h"
#include "Modules/Messages/inc/Msg_Param.h"
#include <string.h>

struct paramHander
{
    eventHandler_t*       evHandler;
    param_obj_t*          rootParam;
    uint8_t               master;
    param_helper_t        helper;

};

uint8_t ParamHander_HandleParamMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

uint8_t ParamHander_HandleParamSaveMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

uint8_t ParamHander_HandleParamLoadMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

paramHander_t* ParamHander_CreateObj(uint8_t num_children, eventHandler_t* evHandler,const char *obj_name, uint8_t master)
{
    paramHander_t *obj = pvPortMalloc( sizeof(paramHander_t) );
    if(!obj)
    {
        return NULL;
    }
    obj->evHandler = evHandler;
    obj->rootParam = Param_CreateObj(num_children, variable_type_NoType, readOnly, NULL, obj_name, NULL, NULL );
    obj->master = master;
    obj->helper = (param_helper_t){0}; // TODO allowed??...
    if(!obj->rootParam)
    {
        return NULL;
    }
    Event_RegisterCallback(obj->evHandler, Msg_Param_e, ParamHander_HandleParamMsg, obj);

    memset(obj->helper.dumpStart, 0, MAX_DEPTH);
    obj->helper.depth = 0;
    obj->helper.sequence = 0;

    if(obj->master)
    {
        Event_RegisterCallback(obj->evHandler, Msg_ParamSave_e, ParamHander_HandleParamMsg, obj);
        Event_RegisterCallback(obj->evHandler, Msg_ParamLoad_e, ParamHander_HandleParamMsg, obj);
    }
    return obj;
}
void ParamHander_deleteHandler(paramHander_t *obj)
{
    Param_DeleteObj(obj->rootParam);
    vPortFree(obj);
}

uint8_t ParamHander_Dump(paramHander_t *obj, uint8_t *buffer, uint32_t buffer_length, param_helper_t *helper)
{
    buffer[0] = '\0';
    if(!Param_AppendDivider( buffer, buffer_length))
    {
        return 0;
    }
    helper->depth = 0;
    return Param_AppendDumpFromHere(obj->rootParam, buffer, buffer_length, helper);
}

param_obj_t* ParamHandler_GetParam(paramHander_t* obj)
{
    return obj ? obj->rootParam : NULL;
}


uint8_t ParamHander_HandleParamMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    uint8_t result = 0;
    paramHander_t* handlerObj = (paramHander_t*)data; // The data should always be the handler object when a Msg_param is received.
    if(!handlerObj)
    {
        return 0;
    }


    uint8_t control = Msg_ParamGetControl(msg);

    // Parameter messages uses string payload, and they have persistent storage, and has to
    // be manually allocated and freed. This means we can re-use the buffer provided in this
    // message and send it back to the caller.
    uint8_t* payload = Msg_ParamGetPayload(msg);
    uint32_t payloadLength = Msg_ParamGetPayloadlength(msg);
    uint32_t payloadBufferLength = Msg_ParamGetPayloadbufferlength(msg);
    switch (control)
    {
    case param_set:
    {
        result = Param_SetFromRoot(handlerObj->rootParam,payload, payloadLength);
        if(!result)
        {
            //TODO send error setting parameters.
            //Event_Send();// TODO...
        }
    }
    break;
    case param_get:
    {
        payload[0] = '\0';
        uint8_t result_get = 0;
        result_get = ParamHander_Dump(handlerObj, payload, payloadBufferLength, &handlerObj->helper);
        uint8_t lastInSequence = 0;
        payloadLength = strlen((char*)payload);
        if(result_get) // Dump from root is finished, reset helper.
        {
          memset(handlerObj->helper.dumpStart, 0, MAX_DEPTH); // Starting point of dump.
          handlerObj->helper.sequence = 0;
          lastInSequence = 1;
        } // else dump is not finished, keep helper till next time.
        else
        {
          handlerObj->helper.sequence++;
        }

        moduleMsg_t* paramMsg = Msg_ParamCreate(Msg_GetSource(msg),0,param_get,handlerObj->helper.sequence,
                lastInSequence, payload, payloadLength, payloadBufferLength);
        Event_Send(handlerObj->evHandler,paramMsg);
        result = 1;
    }
    break;
    }

    return result;
}
uint8_t ParamHander_HandleParamSaveMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    uint8_t result = 0;
    paramHander_t* handlerObj = (paramHander_t*)data; // The data should always be the handler object when a Msg_paramSave is received.
    if(!handlerObj->master)
    {
        return 0;
    }
    //TODO save parameters...
    return 0;
}

uint8_t ParamHander_HandleParamLoadMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    uint8_t result = 0;
    paramHander_t* handlerObj = (paramHander_t*)data; // The data should always be the handler object when a Msg_paramLoad is received.
    if(!handlerObj->master)
    {
        return 0;
    }
    //TODO load paramters...
    return 0;
}

