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
#include "Messages/inc/Msg_ParamFc.h"
#include <string.h>
#include "Utilities/inc/string_utils.h"
#include "BoardConfig.h"

#include "FreeRTOS.h"
#include "task.h"
//#define DEBUG

#include "Components/AppLog/inc/AppLog.h"


#define MAX_PARAM_HANDLERS (8)
#define PARAM_BUFFER_LEN (200)
#define SLIP_PACKET_LEN (PARAM_BUFFER_LEN + 60)

typedef enum
{
    paramReady,
    paramActionOngoing,
} paramAction_t;

struct paramHander
{
    eventHandler_t*       evHandler;
    param_obj_t*          rootParam;
    param_helper_t        helper;
    param_helper_t        saveHelper;
};

/**
 * Message handler used by all ParamHandlers. This handler will be used to answer any query made
 * by the master ParamHandler.
 * @param obj
 * @param data
 * @param msg
 * @return
 */
uint8_t ParamHandler_HandleParamFcMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

/**
 * Answer if the ParamHandler has any parameters.Only used internally. Only master ParamHandler sends this.
 * @param obj
 * @param data
 * @param msg
 * @return
 */
uint8_t ParamHandler_HandleHasParamMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

paramHander_t* ParamHandler_CreateObj(uint8_t num_children, eventHandler_t* evHandler,const char *obj_name, uint8_t master)
{
    paramHander_t *obj = pvPortMalloc( sizeof(paramHander_t) );
    if(!obj)
    {
        return NULL;
    }
    obj->evHandler = evHandler;
    obj->rootParam = Param_CreateObj(num_children, variable_type_NoType, readOnly, NULL, obj_name, NULL);
  
    if(!obj->rootParam)
    {
        return NULL;
    }
    memset(obj->helper.dumpStart, 0, MAX_DEPTH);
    obj->helper.depth = 0;
    obj->helper.sequence = 0;

    memset(obj->saveHelper.dumpStart, 0, MAX_DEPTH);
    obj->saveHelper.depth = 0;
    obj->saveHelper.sequence = 0;

    if(obj->evHandler)
    {
        Event_RegisterCallback(obj->evHandler, Msg_HasParam_e, ParamHandler_HandleHasParamMsg, obj);
        Event_RegisterCallback(obj->evHandler, Msg_ParamFc_e, ParamHandler_HandleParamFcMsg, obj);
    }

    return obj;
}

void ParamHandler_DeleteHandler(paramHander_t *obj)
{
    if(obj->rootParam)
    {
        Param_DeleteObj(obj->rootParam);
    }
    vPortFree(obj);
}


uint8_t ParamHandler_Dump(paramHander_t *obj, uint8_t *buffer, uint32_t buffer_length, param_helper_t *helper)
{
    buffer[0] = '\0';
    if(!Param_AppendDivider( buffer, buffer_length))
    {
        return 0;
    }
    helper->depth = 0;
    return Param_AppendDumpFromHere(obj->rootParam, buffer, buffer_length, helper);
}


uint8_t ParamHandler_SetFromRoot(paramHander_t *obj, uint8_t *buffer, uint32_t buffer_length)
{
    if(!obj || !buffer || !buffer_length)
    {
        return 0;
    }
    // First char of the buffer has to indicate that the path is relative root.
    if(buffer[0] != '/')
    {
        return 0;
    }
    uint8_t * buffPtr = (buffer + 1);


    return Param_SetFromHere(obj->rootParam, buffPtr, buffer_length-1);
}

param_obj_t* ParamHandler_GetParam(paramHander_t* obj)
{
    return obj ? obj->rootParam : NULL;
}

uint8_t * ParamHandler_FindRoot(paramHander_t *obj, uint8_t *Buffer, uint32_t BufferLength)
{
    if(!obj || !Buffer || !BufferLength)
    {
        return NULL;
    }
    // First char of the buffer has to indicate that the path is relative root.
    if(Buffer[0] != '/')
    {
        return NULL;
    }
    // Find the first root param that has our name.
    uint8_t* beginingOfModulePtr = Buffer+1;
    uint32_t remainingLength = BufferLength-1;
    uint32_t depth = 0;
    uint8_t* result = NULL;
    while((remainingLength > 0))
    {
        // Loop until we either run out of buffer, or find the correct param.
        uint8_t moduleBuffer[MAX_PARAM_NODE_LENGTH];

        //Get the module string.
        uint8_t* buffPtr = FcString_GetModuleString(moduleBuffer, MAX_PARAM_NODE_LENGTH, beginingOfModulePtr, remainingLength);
        if(!buffPtr)
        {
            break;
        }
        remainingLength = (uint32_t)(BufferLength - (buffPtr - Buffer));


        if(depth == 0) // If we are at root level, check the name of the current root parameter.
        {

            uint8_t bufferName[MAX_PARAM_NAME_LENGTH+1];

            bufferName[0] = '\0';

            // Make sure the string is null terminated.
            bufferName[MAX_PARAM_NAME_LENGTH] = '\0';
            if(!FcString_GetNameString(bufferName, MAX_PARAM_NAME_LENGTH, moduleBuffer, MAX_PARAM_NODE_LENGTH))
            {
                return 0;
            }

            if(0 == strncmp((char*)bufferName,(char*)obj->rootParam->group_name,  MAX_PARAM_NAME_LENGTH))
            {
                result = beginingOfModulePtr;
                break;
            }

        }
        if(strncmp((char*)moduleBuffer, "..",2) == 0)
        {
            depth--; //Decrease depth if we find the parent notation.
        }
        else
        {
            depth++; // increase depth if we did not find the current name, and also not the parent notation.
        }
        beginingOfModulePtr = buffPtr;
    }
    return result;
}

uint8_t ParamHandler_HandleParamFcMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    uint8_t result = 0;
    paramHander_t* handlerObj = (paramHander_t*)data; // The data should always be the handler object when a Msg_param is received.
//    LOG_DBG_ENTRY(FC_SerialIOtx_e, obj, "%s Received param message from %s.", getStrFromAddr(obj->handlerId), getStrFromAddr(Msg_GetSource(msg)));
    uint8_t control = Msg_ParamFcGetControl(msg);

    switch (control)
    {
    case param_set:
    {
        uint8_t* payload = Msg_ParamFcGetPayload(msg);
        uint32_t payloadLength = Msg_ParamFcGetPayloadlength(msg);
        uint8_t* root = NULL;
        root = ParamHandler_FindRoot(handlerObj, payload, payloadLength);
        if(root)
        {
            uint32_t remainingLength = (uint32_t)(payloadLength - (root - payload));

            //First, set locally.
            result = Param_SetFromHere(handlerObj->rootParam, root, remainingLength);
            if(!result)
            {
                //TODO send error setting parameters.
            }
        }

        // Send answer to the paramHandler master. Set messages to the master has no length as they are only used for signaling.
        moduleMsg_t* paramMsg = Msg_ParamFcCreate(Msg_GetSource(msg),0,param_set,handlerObj->helper.sequence, 0, 0); // TODO not needed?
        Event_Send(handlerObj->evHandler,paramMsg);
    }
    break;
    case param_get:
    case param_save:
    {
        LOG_DBG_ENTRY(FC_SerialIOtx_e, obj, "%s Received save/get param.", getStrFromAddr(obj->handlerId));
        param_helper_t* helper = NULL;
        if(control == param_get)
        {
            helper = &handlerObj->helper;
        }
        else
        {
            helper = &handlerObj->saveHelper;
        }
        // allocate the memory needed for the message, and populate that. We avoid memcpy by creating a message directly.
        moduleMsg_t* paramMsg = Msg_ParamFcCreate(Msg_GetSource(msg),0,control ,0 , 0, PARAM_BUFFER_LEN);
        uint8_t* payload = Msg_ParamFcGetPayload(paramMsg);

        payload[0] = '\0';
        uint8_t result_get = 0;
        result_get = ParamHandler_Dump(handlerObj, payload, PARAM_BUFFER_LEN, helper);
        uint8_t lastInSequence = 0;
        uint32_t payloadLength = strlen((char*)payload);
        if(result_get) // Dump from root is finished, reset helper.
        {
            LOG_DBG_ENTRY(FC_SerialIOtx_e, obj, "%s Param dump finished.", getStrFromAddr(obj->handlerId));
            memset(helper->dumpStart, 0, MAX_DEPTH); // Starting point of dump.
            //handlerObj->helper.sequence = 0;
            lastInSequence = 1;
        } // else dump is not finished, keep helper till next time.
        else
        {
            helper->sequence++;
        }
        Msg_ParamFcSetPayloadlength(paramMsg, payloadLength);
        Msg_ParamFcSetLastinsequence(paramMsg, lastInSequence);
        Msg_ParamFcSetSequencenr(paramMsg, helper->sequence);

        // send the reply to the caller, should always be the master ParamHandler.
        LOG_DBG_ENTRY(FC_SerialIOtx_e, obj, "%s Sending msg to master", getStrFromAddr(obj->handlerId));
        Event_Send(handlerObj->evHandler,paramMsg);
        result = 1;
    }
    break;
    }

    return result;
}

uint8_t ParamHandler_HandleHasParamMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || ! data || !msg)
    {
        return 0;
    }
    if(Msg_GetDestination(msg) != FC_Broadcast_e)
    {
        return 0; // Handlers should only answer broadcasts of this type.
    }
    // Handlers will reply that they do have parameters.
    moduleMsg_t* reply = Msg_Create(Msg_HasParam_e, Msg_GetSource(msg));
    Event_Send(obj, reply);
    return 1;
}

