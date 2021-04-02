/*
* paramMasterHandler.c
*
* Copyright (C) 2021  Martin Lundh
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

#include "../inc/paramMasterHandler.h"
#include "Messages/inc/Msg_ParamFc.h"
#include "Messages/inc/Msg_Param.h"
#include <string.h>
#include "Utilities/inc/string_utils.h"
#include "Components/SLIP/inc/slip_packet.h"
#include "HAL/QuadFC/QuadFC_Memory.h"
#include "BoardConfig.h"

#include "FreeRTOS.h"
#include "task.h"
// #define DEBUG

#include "Components/AppLog/inc/AppLog.h"

#define MAX_PARAM_HANDLERS (8)
#define PARAM_BUFFER_LEN (200)
#define SLIP_PACKET_LEN (PARAM_BUFFER_LEN + 60)

typedef enum
{
    paramReady,
    paramActionOngoing,
} paramAction_t;

struct ParamMaster
{
    eventHandler_t *evHandler;
    msgAddr_t *handlers;
    uint8_t nrRegisteredHandlers;
    uint32_t currentlyGetting;
    uint32_t currentlySavinging;
    msgAddr_t getter;
    msgAddr_t saver;
    uint8_t sequenceGet;
    uint8_t sequenceSet;
    uint8_t saveSequence;
    uint32_t setAddress;
    paramAction_t actionOngoing;
    SLIP_Parser_t *parser;
};

void printPacket(eventHandler_t *obj, SLIP_t *packet)
{ // TODO!!
#ifdef DEBUG
    uint8_t array[400] = {0};
    for (int i = 0; i < packet->packetSize; i++)
    {
        if ((packet->payload[i] >= 33) && (packet->payload[i] <= 127))
        {
            snprintf((char *)array + i, 2, "%c", (char)packet->payload[i]);
        }
        else
        {
            snprintf((char *)array + i, 2, "%c", '0');
        }
    }
    LOG_DBG_ENTRY( obj, "Packet: %.*s", (int)220, array);
#endif
}

/**
 * Only the master paramHandler will use this message handler. It will be the only entry point from the
 * outside to this tree of parameters.
 *
 * All external queries should be of ParamMsg type, and they should be sent to the master ParamHandler.
 * The master paramHandler will then internally communicate with the other handlers, and answer the originator
 * accordingly.
 * @param obj
 * @param data
 * @param msg
 * @return
 */
uint8_t ParamMaster_HandleParamMsg(eventHandler_t *obj, void *data, moduleMsg_t *msg);

/**
  * Message handler used only by the paramHandlerMaster. This handler will be used when other handlers are
  * sending information as replies to the messages the master has sent. This message type is only used internally in the FC.
  * @param obj
  * @param data
  * @param msg
  * @return
  */
uint8_t ParamMaster_HandleParamFcMsg(eventHandler_t *obj, void *data, moduleMsg_t *msg);

/**
 * Answer if the ParamHandler has any parameters.Only used internally. 
 * @param obj
 * @param data
 * @param msg
 * @return
 */
uint8_t ParamMaster_HandleHasParamMsg(eventHandler_t *obj, void *data, moduleMsg_t *msg);

ParamMaster_t *ParamMaster_CreateObj(eventHandler_t *evHandler)
{
    ParamMaster_t *obj = pvPortMalloc(sizeof(ParamMaster_t));
    if (!obj)
    {
        return NULL;
    }
    obj->evHandler = evHandler;
    obj->nrRegisteredHandlers = 0;
    obj->currentlyGetting = 0;
    obj->currentlySavinging = 0;
    obj->getter = Unassigned_e;
    obj->saver = Unassigned_e;
    obj->sequenceGet = 0;
    obj->sequenceSet = 0;
    obj->saveSequence = 0;
    obj->setAddress = PARAM_MEM_START_ADDR;
    obj->actionOngoing = paramReady;
    obj->parser = SlipParser_Create(SLIP_PACKET_LEN);
    if (!obj->parser)
    {
        return NULL;
    }
    obj->nrRegisteredHandlers = 0;

    obj->handlers = NULL;
    if (obj->evHandler)
    {
        obj->handlers = pvPortMalloc(sizeof(msgAddr_t) * MAX_PARAM_HANDLERS);
        Event_RegisterCallback(obj->evHandler, Msg_HasParam_e, ParamMaster_HandleHasParamMsg, obj);
        Event_RegisterCallback(obj->evHandler, Msg_Param_e, ParamMaster_HandleParamMsg, obj);
        Event_RegisterCallback(obj->evHandler, Msg_ParamFc_e, ParamMaster_HandleParamFcMsg, obj);
    }

    return obj;
}

void ParamMaster_DeleteHandler(ParamMaster_t *obj)
{
    if (obj->handlers)
    {
        vPortFree(obj->handlers);
    }
    if (obj->parser)
    {
        SlipParser_Delete(obj->parser);
    }
    vPortFree(obj);
}

uint8_t ParamMaster_InitMaster(ParamMaster_t *obj)
{
    if (!obj)
    {
        return 0;
    }
    moduleMsg_t *msg = Msg_Create(Msg_HasParam_e, FC_Broadcast_e);
    Event_Send(obj->evHandler, msg);
    return 1;
}

// External messages
uint8_t ParamMaster_HandleParamMsg(eventHandler_t *obj, void *data, moduleMsg_t *msg)
{
    if (!obj || !data || !msg)
    {
        return 0;
    }
    uint8_t result = 0;
    ParamMaster_t *handlerObj = (ParamMaster_t *)data; // The data should always be the handler object when a Msg_param is received.

    //If we are busy and the message is not from ourselfs, we must block the call.
    if (handlerObj->actionOngoing != paramReady && Msg_GetSource(msg) != obj->handlerId)
    {
        moduleMsg_t *msgParam = Msg_ParamCreate(Msg_GetSource(msg), 0, param_error, 0, 0, 0);
        Event_Send(obj, msgParam);
        LOG_ENTRY(obj, "Param: Action ongoing, discarding message.");
        return 1;
    }
    handlerObj->actionOngoing = paramActionOngoing; // Start action.

    uint8_t control = Msg_ParamGetControl(msg);
    uint32_t payloadLength = Msg_ParamGetPayloadlength(msg);
    uint8_t *payload = Msg_ParamGetPayload(msg);

    switch (control)
    {
    case param_set:
    {
        //Send to every paramHandler. We copy the message payload.
        for (int i = 0; i < handlerObj->nrRegisteredHandlers; i++)
        {
            LOG_DBG_ENTRY( obj, "%s Sending ParamSet to %s", getStrFromAddr(obj->handlerId), getStrFromAddr(handlerObj->handlers[i]));

            moduleMsg_t *msgParam = Msg_ParamFcCreate(handlerObj->handlers[i], 0, param_set, 0, 0, payloadLength);
            if (!msgParam)
            {
                LOG_ENTRY(obj, "Param Set: Error - Not able to create a paramFC message.");
                handlerObj->actionOngoing = paramReady; // Reset.
                break;
            }
            Msg_ParamFcSetPayloadlength(msgParam, payloadLength);

            uint8_t *paramPayload = Msg_ParamFcGetPayload(msgParam);
            memcpy(paramPayload, payload, payloadLength);

            Event_Send(obj, msgParam);
        }
        handlerObj->actionOngoing = paramReady; // Set is ready once we have handled the message.
    }
    break;
    case param_load:
    {
        uint8_t lastInSequence = 0;
        uint8_t EcpectedSequenceNo = 0;
        uint32_t address = PARAM_MEM_START_ADDR;
        LOG_DBG_ENTRY( obj, "%s Received ParamLoad.", getStrFromAddr(obj->handlerId));

        while (!lastInSequence)
        {
            uint8_t buffer[2];
            SLIP_Status_t read_status = SLIP_StatusCont;
            uint32_t startAddress = address;
            SLIP_t *packet = Slip_Create(SLIP_PACKET_LEN);

            // Read from memory and add to the parser.
            while (SLIP_StatusCont == read_status)
            {
                result = Mem_Read(address, 1, buffer, 2);
                if (result && ((address - startAddress) <= SLIP_PACKET_LEN))
                {
                    read_status = SlipParser_Parse(handlerObj->parser, buffer, 1, packet);
                }
                else
                {
                    result = 0;
                    Slip_Delete(packet); // Cleanup...
                    LOG_ENTRY(obj, "Param Load: Packet could not fit in buffer.");
                    handlerObj->actionOngoing = paramReady; // Reset.
                    break;
                }
                address++;
            }
            if (read_status == SLIP_StatusNok)
            {
                LOG_ENTRY(obj, "Param Load: Error. Parser failed.");
            }
            LOG_DBG_ENTRY( obj, "Param Load: Read %d at %d", (int)(address - startAddress), (int)startAddress);
            printPacket(obj, packet);
            // Whole package or error...
            result &= (read_status == SLIP_StatusOK);

            moduleMsg_t *loadedMsg = NULL;

            if (result)
            {
                loadedMsg = Msg_ParamFcDeserialize(packet->payload, packet->packetSize);

                uint8_t sequenceNo = Msg_ParamFcGetSequencenr(loadedMsg);
                lastInSequence = Msg_ParamFcGetLastinsequence(loadedMsg);
                result = (sequenceNo == EcpectedSequenceNo);
                EcpectedSequenceNo++;
            }
            else
            {
                LOG_ENTRY(obj, "Param Load: Error. Deserialize failed.");
                handlerObj->actionOngoing = paramReady; // Reset.
                break;
            }

            if (result) // if ok, send to self to handle the message.
            {
                Event_Send(obj, loadedMsg);
            }
            else // Read was not successful.
            {
                Msg_Delete(&loadedMsg);
                Slip_Delete(packet);
                LOG_ENTRY(obj, "Param Load: Error. Failed to read param.");
                handlerObj->actionOngoing = paramReady; // Reset.
                break;
            }
            Slip_Delete(packet);
        }
        LOG_DBG_ENTRY( obj, "Param Load: Finished.");
        handlerObj->actionOngoing = paramReady; // Load is ready when all messages has been sent.
    }
    break;
    case param_get:
    {
        handlerObj->getter = Msg_GetSource(msg); // save who is currently getting the tree.
        if ((handlerObj->currentlyGetting) < handlerObj->nrRegisteredHandlers)
        {
            LOG_DBG_ENTRY( obj, "Param: Master Request from other handlers.");

            moduleMsg_t *msgParam = Msg_ParamFcCreate(handlerObj->handlers[(handlerObj->currentlyGetting)],
                                                      0, control, 0, 0, 0);
            Event_Send(obj, msgParam);
        }
    }
    break;
    case param_save:
    {
        if ((handlerObj->currentlySavinging) < handlerObj->nrRegisteredHandlers)
        {
            handlerObj->saver = Msg_GetSource(msg); // save who is currently saving the tree.

            moduleMsg_t *msgParam = Msg_ParamFcCreate(handlerObj->handlers[(handlerObj->currentlySavinging)],
                                                      0, control, 0, 0, 0);
            Event_Send(obj, msgParam);
        }
    }
    break;
    }
    return result;
}

//Internal messages.
uint8_t ParamMaster_HandleParamFcMsg(eventHandler_t *obj, void *data, moduleMsg_t *msg)
{
    if (!obj || !data || !msg)
    {
        return 0;
    }
    uint8_t result = 0;
    ParamMaster_t *handlerObj = (ParamMaster_t *)data; // The data should always be the handler object when a Msg_param is received.

    uint8_t control = Msg_ParamFcGetControl(msg);

    // Parameter messages uses string payload, and they have persistent storage, and has to
    // be manually allocated and freed. This means we can re-use the buffer provided in this
    // message and send it back to the caller.
    uint8_t *payload = Msg_ParamFcGetPayload(msg);
    uint32_t payloadLength = Msg_ParamFcGetPayloadlength(msg);
    switch (control)
    {
    case param_set:
    {
        //nothing to do for set, used for signaling only.
    }
    break;
    case param_get:
    {
        LOG_DBG_ENTRY( obj, "ParamFC Master: %s Received Get from %s.", getStrFromAddr(obj->handlerId), getStrFromAddr(Msg_GetSource(msg)));
        uint8_t lastInSequence = 0;
        if (Msg_ParamFcGetLastinsequence(msg))
        {
            handlerObj->currentlyGetting++; //Last of the current handler, move on to the next.
            if (handlerObj->currentlyGetting >= handlerObj->nrRegisteredHandlers) // last of all handlers, signal last in sequence.
            {
                LOG_DBG_ENTRY( obj, "ParamFC Master: %s Last in sequense!", getStrFromAddr(obj->handlerId));

                lastInSequence = 1;
                handlerObj->currentlyGetting = 0;
            }
        }

        // We have to create a new message as it has a different type. Copy the data.
        moduleMsg_t *paramMsg = Msg_ParamCreate(handlerObj->getter, 0, param_set, handlerObj->sequenceGet, lastInSequence, payloadLength);
        uint8_t *paramPayload = Msg_ParamGetPayload(paramMsg);
        memcpy(paramPayload, payload, payloadLength);

        Msg_ParamSetPayloadlength(paramMsg, payloadLength);
        Event_Send(handlerObj->evHandler, paramMsg);
        result = 1;
        handlerObj->actionOngoing = paramReady; // Reset, ready. We get a new request for each message, so we do not need to block any longer.
        handlerObj->sequenceGet++;
        if(lastInSequence)
        {
            handlerObj->sequenceGet = 0;
        }
    }
    break;
    case param_save:
    {
        uint8_t lastInSequence = 0;
        if (Msg_ParamFcGetLastinsequence(msg))
        {
            handlerObj->currentlySavinging++;                                       //Last of the current handler, move on to the next.
            if (handlerObj->currentlySavinging >= handlerObj->nrRegisteredHandlers) // last of all handlers, signal last in sequence.
            {
                LOG_DBG_ENTRY( obj, "Last in sequense!");

                lastInSequence = 1;
                handlerObj->currentlySavinging = 0;
                handlerObj->actionOngoing = paramReady; // Reset, ready.
            }
        }

        // Save the parameters collected from this handler.
        Msg_ParamFcSetLastinsequence(msg, lastInSequence); // Set a global lastInSequence to help the load function.
        Msg_ParamFcSetControl(msg, param_set);
        Msg_SetDestination(msg, Msg_GetSource(msg));// Set source as destination so that we can return the data to the source once we load it.
        Msg_ParamFcSetSequencenr(msg, handlerObj->saveSequence);
        uint8_t serializeBuffer[SLIP_PACKET_LEN];
        uint32_t serializebufferSize = SLIP_PACKET_LEN;

        uint8_t *buffP = Msg_ParamFcSerialize(msg, serializeBuffer, serializebufferSize);

        uint32_t serializedLength = buffP - serializeBuffer;
        SLIP_t *packet = Slip_Create(SLIP_PACKET_LEN);

        // Packetize into a slip package before write to mem.
        result = Slip_Packetize(serializeBuffer, serializedLength, SLIP_PACKET_LEN, packet);
        result = result && ((handlerObj->setAddress + packet->packetSize) < PARAM_MEM_STOP_ADDR);
        if (result) // Packetization OK, write to mem.
        {
            LOG_DBG_ENTRY( obj, "Param Save: Saving %d at %d", (int)(packet->packetSize), (int)handlerObj->setAddress);
            printPacket(obj, packet);
            
            result = Mem_Write(handlerObj->setAddress, packet->packetSize,
                               packet->payload, packet->allocatedSize);
            handlerObj->setAddress += (packet->packetSize + 1);
        }
        else
        {
            LOG_ENTRY(obj, "Not able to write param to memory, address: %ld.", handlerObj->setAddress)
            break;
        }
        
        Slip_Delete(packet);
        //LOG_DBG_ENTRY( obj, "Currently saving: %ld", handlerObj->currentlySavinging);

        if (!lastInSequence) // Send to next handler.
        {
            moduleMsg_t *msgParam = Msg_ParamFcCreate(handlerObj->handlers[(handlerObj->currentlySavinging)],
                                                      0, control, 0, 0, 0);
            Event_Send(obj, msgParam);
            handlerObj->saveSequence++;
        }
        else
        {
            handlerObj->saveSequence = 0;
            handlerObj->setAddress = PARAM_MEM_START_ADDR;
        }       
    }
    break;
    }
    if(!result)
    {
        // Clean-up
        handlerObj->actionOngoing = paramReady; 
        handlerObj->saveSequence = 0;
        handlerObj->setAddress = PARAM_MEM_START_ADDR;
        handlerObj->currentlySavinging = 0;
    }
    return result;
}

uint8_t ParamMaster_HandleHasParamMsg(eventHandler_t *obj, void *data, moduleMsg_t *msg)
{
    if (!obj || !data || !msg)
    {
        return 0;
    }
    ParamMaster_t *handlerObj = (ParamMaster_t *)data; // The data should always be the handler object when a Msg_paramSave is received.

    if (Msg_GetDestination(msg) == FC_Broadcast_e)
    {
        return 0; // master does not answer to broadcasts of this type.
    }
    // Save the address of the sender, this address has a paramHandler.
    if (handlerObj->nrRegisteredHandlers >= MAX_PARAM_HANDLERS)
    {
        configASSERT(0);
    }
    handlerObj->handlers[handlerObj->nrRegisteredHandlers++] = Msg_GetSource(msg);

    return 1;
}
