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
#include "Modules/Messages/inc/Msg_ParamFc.h"
#include "Modules/Messages/inc/Msg_Param.h"
#include <string.h>
#include "Modules/Utilities/inc/string_utils.h"
#include "Components/SLIP/inc/slip_packet.h"
#include "HAL/QuadFC/QuadFC_Memory.h"

#define MAX_PARAM_HANDLERS (8)
#define PARAM_BUFFER_LEN (200)
#define SLIP_PACKET_LEN (PARAM_BUFFER_LEN + 10)
struct paramHander
{
    eventHandler_t*       evHandler;
    param_obj_t*          rootParam;
    uint8_t               master;
    param_helper_t        helper;
    param_helper_t        saveHelper;
    msgAddr_t*            handlers; //!< Addresses to all slave handlers. Only used by master.
    uint8_t               nrRegisteredHandlers;
    uint32_t              currentlyGetting;
    msgAddr_t             getter;
    uint8_t               sequence;
    uint8_t               setAddress;

};

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
 */uint8_t ParamHandler_HandleParamMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

 /**
  * Message handler used only by the paramHandlerMaster. This handler will be used when other handlers are
  * sending information as replies to the messages the master has sent. This message type is only used internally in the FC.
  * @param obj
  * @param data
  * @param msg
  * @return
  */
 uint8_t ParamHandler_HandleParamFcMsgMaster(eventHandler_t* obj, void* data, moduleMsg_t* msg);

 /**
  * Message handler used by all ParamHandlers except master. This handler will be used to answer any query made
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
     obj->master = master;
     obj->helper = (param_helper_t){0};
     obj->nrRegisteredHandlers = 0;
     obj->currentlyGetting = 0;
     obj->getter = Unassigned;
     obj->sequence = 0;
     obj->setAddress = 0;
     if(!obj->rootParam)
     {
         return NULL;
     }
     obj->nrRegisteredHandlers = 0;

     memset(obj->helper.dumpStart, 0, MAX_DEPTH);
     obj->helper.depth = 0;
     obj->helper.sequence = 0;

     memset(obj->saveHelper.dumpStart, 0, MAX_DEPTH);
     obj->saveHelper.depth = 0;
     obj->saveHelper.sequence = 0;

     obj->handlers = NULL;
     if(obj->evHandler)
     {
         Event_RegisterCallback(obj->evHandler, Msg_HasParam_e, ParamHandler_HandleHasParamMsg, obj);

         if(obj->master)
         {
             obj->handlers = pvPortMalloc(sizeof(msgAddr_t)*MAX_PARAM_HANDLERS);
             Event_RegisterCallback(obj->evHandler, Msg_Param_e, ParamHandler_HandleParamMsg, obj);
             Event_RegisterCallback(obj->evHandler, Msg_ParamFc_e, ParamHandler_HandleParamFcMsgMaster, obj);
         }
         else
         {
             obj->handlers = NULL;
             Event_RegisterCallback(obj->evHandler, Msg_ParamFc_e, ParamHandler_HandleParamFcMsg, obj);
         }
     }

     return obj;
 }

 void ParamHandler_DeleteHandler(paramHander_t *obj)
 {
     if(obj->rootParam)
     {
         Param_DeleteObj(obj->rootParam);
     }
     if(obj->handlers)
     {
         vPortFree(obj->handlers);
     }
     vPortFree(obj);
 }

 uint8_t ParamHandler_InitMaster(paramHander_t *obj)
 {
     if(!obj || !obj->master)
     {
         return 0;
     }
     moduleMsg_t* msg = Msg_Create(Msg_HasParam_e, Broadcast);
     Event_Send(obj->evHandler, msg);
     return 1;
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

 uint8_t ParamHandler_HandleParamMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
 {
     if(!obj || ! data || !msg)
     {
         return 0;
     }
     uint8_t result = 0;
     paramHander_t* handlerObj = (paramHander_t*)data; // The data should always be the handler object when a Msg_param is received.

     uint8_t control = Msg_ParamGetControl(msg);
     uint8_t* payload = Msg_ParamGetPayload(msg);
     uint32_t payloadLength = Msg_ParamGetPayloadlength(msg);

     switch (control)
     {
     case param_set:
     {

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

         if(!result)
         {

         }
         //Then send to every other paramHandler. We copy the message payload.
         for(int i = 0; i < handlerObj->nrRegisteredHandlers; i++)
         {
             moduleMsg_t* msgParam = Msg_ParamFcCreate(handlerObj->handlers[i],0,param_set,0,0,payloadLength);
             if(!msgParam)
             {
                 return 0;
             }
             Msg_ParamFcSetPayloadlength(msgParam, payloadLength);

             uint8_t* paramPayload = Msg_ParamFcGetPayload(msgParam);
             memcpy(paramPayload, payload, payloadLength);

             Event_Send(obj, msgParam);
         }
     }
     break;
     case param_load:
     {
         uint8_t lastInSequence = 0;
         uint8_t EcpectedSequenceNo = 0;
         uint32_t address = 0;

         while(!lastInSequence)
         {
             int k = 0;
             uint8_t buffer[2];
             SLIP_Status_t read_status = SLIP_StatusCont;
             uint32_t startAddress = address;
             SLIP_t* packet = Slip_Create(SLIP_PACKET_LEN);
             uint8_t unpacked_buffer[PARAM_BUFFER_LEN] = {0};

             // Read from memory and add to the parser.
             while(SLIP_StatusCont == read_status)
             {
                 result = Mem_Read(address, 1, buffer, 2);
                 if(result && ((address - startAddress) < SLIP_PACKET_LEN))
                 {
                     read_status = SLIP_Parser(buffer, 1, packet, &k);
                 }
                 else
                 {
                     result = 0;
                     break;
                 }
                 address++;
             }
             result &= (read_status == SLIP_StatusOK);
             if(result)
             {
                 if(!Slip_DePacketize(unpacked_buffer, PARAM_BUFFER_LEN, packet))
                 {
                     result = 0;
                 }
             }

             if(result)
             {
                 uint8_t sequenceNo = Msg_ParamGetSequencenr(msg);
                 lastInSequence = Msg_ParamFcGetLastinsequence(msg);
                 result = (sequenceNo == EcpectedSequenceNo);
                 EcpectedSequenceNo++;
             }
             if(result)
             {
                 //Then send to every other paramHandler.
                 for(int i = 0; i < handlerObj->nrRegisteredHandlers; i++)
                 {
                     //moduleMsg_t* msgParam = Msg_ParamFcCreate(handlerObj->handlers[i],0,param_set,0,0,payload,payloadLength,payloadBufferLength);
                     //Event_Send(obj, msgParam);
                     //Event_WaitForEvent(obj, Msg_ParamFc_e, 1, 0);
                 }
             }

             else // Read or set was not successful.
             {
                 //TODO log error.
                 break;
             }
             Slip_Delete(packet);
         }
     }
     break;
     case param_get:
     case param_save:

     {
         handlerObj->getter = Msg_GetSource(msg); // save who is currently getting the tree.

         uint32_t bufferLength = PARAM_BUFFER_LEN;
         // Create the message here to use the memory.


         if(handlerObj->currentlyGetting == 0)
         {
             moduleMsg_t* msgReply = Msg_ParamCreate(obj->handlerId, 0, param_set, 0, 0, bufferLength );

             uint8_t* buffer = Msg_ParamGetPayload(msgReply);
             //Get all parameters from the local handler.
             buffer[0] = '\0';
             uint8_t result_get = 0;
             result_get = ParamHandler_Dump(handlerObj, buffer, bufferLength, &handlerObj->helper);
             uint8_t lastInSequence = 0;
             handlerObj->sequence = handlerObj->helper.sequence;

             payloadLength = strlen((char*)buffer);
             if(result_get) // Dump from root is finished in local handler, reset helper.
             {
                 memset(handlerObj->helper.dumpStart, 0, MAX_DEPTH); // Starting point of dump.
                 handlerObj->helper.sequence = 0;
                 // Set last in sequence if we are the only paramHandler in the system.
                 lastInSequence = handlerObj->nrRegisteredHandlers <= 0 ? 1 : 0;
                 handlerObj->currentlyGetting++;
             } // else dump is not finished, keep helper till next time.
             else
             {
                 handlerObj->helper.sequence++;
             }

             if(control == param_get)
             {
                 // send the parameters collected from this handler.
                 // Prep the already created message.
                 Msg_ParamSetControl(msgReply, param_get);
                 // This will set the global sequence to be equal to the master sequence. This is useful
                 // since we always send the master first.
                 Msg_ParamSetSequencenr(msgReply, handlerObj->sequence);
                 Msg_ParamSetLastinsequence(msgReply, lastInSequence);
                 Msg_ParamSetPayloadlength(msgReply, payloadLength);
                 Msg_SetDestination(msgReply, Msg_GetSource(msg));

                 Event_Send(handlerObj->evHandler,msgReply);
                 result = 1;
             }
             else
             {
                 // Save the parameters collected from this handler.
                 Msg_ParamSetControl(msgReply, param_set);
                 // This will set the global sequence to be equal to the master sequence. This is useful
                 // since we always send the master first.
                 Msg_ParamSetSequencenr(msgReply, handlerObj->sequence);
                 Msg_ParamSetLastinsequence(msgReply, lastInSequence);
                 Msg_ParamSetPayloadlength(msgReply, payloadLength);
                 Msg_SetDestination(msgReply, obj->handlerId);

                 SLIP_t* packet = Slip_Create(SLIP_PACKET_LEN);
                 // Packetize into a slip package before write to mem.
                 result = Slip_Packetize(buffer, payloadLength, bufferLength, packet);
                 if(result) // Packetization OK, write to mem.
                 {
                     result = Mem_Write(handlerObj->setAddress, packet->packetSize,
                             packet->payload, packet->allocatedSize);
                     handlerObj->setAddress += (packet->packetSize + 1);
                 }
                 Slip_Delete(packet);

                 Msg_Delete(msgReply); // need to delete ourself since we created but did not send.
             }
         }
         else if(handlerObj->currentlyGetting-1 < handlerObj->nrRegisteredHandlers) // -1 since we are counted as "0".
         {
             moduleMsg_t* msgParam = Msg_ParamFcCreate(handlerObj->handlers[handlerObj->currentlyGetting-1],
                     0,control,0,0,0);
             Event_Send(obj, msgParam);
         }
     }
     break;
     }
     return result;
 }

 uint8_t ParamHandler_HandleParamFcMsgMaster(eventHandler_t* obj, void* data, moduleMsg_t* msg)
 {
     if(!obj || ! data || !msg)
     {
         return 0;
     }
     uint8_t result = 0;
     paramHander_t* handlerObj = (paramHander_t*)data; // The data should always be the handler object when a Msg_param is received.

     uint8_t control = Msg_ParamFcGetControl(msg);

     // Parameter messages uses string payload, and they have persistent storage, and has to
     // be manually allocated and freed. This means we can re-use the buffer provided in this
     // message and send it back to the caller.
     uint8_t* payload = Msg_ParamFcGetPayload(msg);
     uint32_t payloadLength = Msg_ParamFcGetPayloadlength(msg);
     switch (control)
     {
     case param_set:
     {
         //nothing to do for set, used for signaling only.
     }
     break;
     case param_get:
     case param_save:
     {
         uint8_t lastInSequence = 0;
         if(Msg_ParamFcGetLastinsequence(msg))
         {
             handlerObj->currentlyGetting++; //Last of the current handler, move on to the next.
             if(handlerObj->currentlyGetting-1 >= handlerObj->nrRegisteredHandlers) // last of all handlers, signal last in sequence.
             {
                 lastInSequence = 1;
                 handlerObj->currentlyGetting = 0;
             }
         }
         handlerObj->sequence++;

         if(control == param_get)
         {
             // We have to create a new message as it has a different type. Copy the data.
             moduleMsg_t* paramMsg = Msg_ParamCreate(handlerObj->getter,0,param_get,handlerObj->sequence, lastInSequence, payloadLength);
             uint8_t* paramPayload = Msg_ParamGetPayload(paramMsg);
             memcpy(paramPayload, payload, payloadLength);

             Msg_ParamSetPayloadlength(paramMsg, payloadLength);
             Event_Send(handlerObj->evHandler,paramMsg);
             result = 1;
         }
         else
         {
             // TODO save message!
         }

     }
     break;
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
         moduleMsg_t* paramMsg = Msg_ParamFcCreate(Msg_GetSource(msg),0,param_set,handlerObj->helper.sequence, 0, 0);
         Event_Send(handlerObj->evHandler,paramMsg);
     }
     break;
     case param_get:
     case param_save:
     {
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
     paramHander_t* handlerObj = (paramHander_t*)data; // The data should always be the handler object when a Msg_paramSave is received.
     if(handlerObj->master)
     {
         if(Msg_GetDestination(msg) == Broadcast)
         {
             return 0; // master does not answer to broadcasts of this type.
         }
         // Save the address of the sender, this address has a paramHandler.
         handlerObj->handlers[handlerObj->nrRegisteredHandlers++] = Msg_GetSource(msg);
     }
     else
     {
         if(Msg_GetDestination(msg) != Broadcast)
         {
             return 0; // Handlers that are not master should only answer broadcasts of this type.
         }
         // Handlers that are not master will reply that they do have parameters.
         moduleMsg_t* reply = Msg_Create(Msg_HasParam_e, Msg_GetSource(msg));
         Event_Send(handlerObj->evHandler, reply);
     }

     return 1;
 }
// uint8_t ParamHandler_HandleParamSaveMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
// {
//     if(!obj || ! data || !msg)
//     {
//         return 0;
//     }
//     uint8_t result = 0;
//     paramHander_t* handlerObj = (paramHander_t*)data; // The data should always be the handler object when a Msg_paramSave is received.
//     if(!handlerObj->master) // Only the master is allowed to save data.
//     {
//         return 0;
//     }
//
//     uint8_t cont = 1;
//     uint32_t address = 0;
//     memset(handlerObj->saveHelper->dumpStart, 0, MAX_DEPTH);
//     handlerObj->saveHelper->depth = 0;
//     handlerObj->saveHelper->sequence = 0;
//     uint8_t buffer[PARAM_BUFFER_LEN] = {0};
//     uint8_t lastInSequence = 0;
//     while(!lastInSequence)
//     {
//         uint8_t sequence = handlerObj->saveHelper->sequence;
//
//         uint32_t payloadLength = 0;
//         uint8_t result = ParamHandler_Get(obj, buffer, PARAM_BUFFER_LEN, handlerObj->saveHelper, & lastInSequence, &payloadLength);
//
//         moduleMsg_t* msg = Msg_ParamCreate(obj->handlerId, 0, param_set, sequence, lastInSequence, buffer, payloadLength,PARAM_BUFFER_LEN);
//
//
//         if(result)
//         {
//             SLIP_t* packet = Slip_Create(PARAM_BUFFER_LEN + 10);
//             // Packetize into a slip package before write to mem.
//             result = Slip_Packetize(buffer, payloadLength, PARAM_BUFFER_LEN, packet);
//             if(result) // Packetization OK, write to mem.
//             {
//                 result = Mem_Write(address, packet->packetSize,
//                         packet->payload, packet->allocatedSize);
//                 address += (packet->packetSize + 1);
//             }
//             Slip_Delete(packet);
//         }
//
//         Msg_Delete(msg);
//     }// while(cont)
//     return result;
// }
//
// uint8_t ParamHandler_HandleParamLoadMsg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
// {
//     if(!obj || ! data || !msg)
//     {
//         return 0;
//     }
//     uint8_t result = 0;
//     paramHander_t* handlerObj = (paramHander_t*)data; // The data should always be the handler object when a Msg_paramLoad is received.
//     if(!handlerObj->master)
//     {
//         return 0;
//     }
//     uint8_t lastInSequence = 0;
//     uint8_t EcpectedSequenceNo = 0;
//     uint32_t address = 0;
//
//     while(!lastInSequence)
//     {
//         int k = 0;
//         uint8_t buffer[2];
//         SLIP_Status_t read_status = SLIP_StatusCont;
//         uint8_t result = 0;
//         uint32_t startAddress = address;
//         // Read from memory and add to the parser.
//         while(SLIP_StatusCont == read_status)
//         {
//             result = Mem_Read(address, 1, buffer, 2);
//             if(result && ((address - startAddress) < COM_PACKET_LENGTH_MAX))
//             {
//                 read_status = SLIP_Parser(buffer, 1, obj->SLIP, &k);
//             }
//             else
//             {
//                 result = 0;
//                 break;
//             }
//             address++;
//         }
//         result &= (read_status == SLIP_StatusOK);
//         if(result){
//             if(!Slip_DePacketize(/*payload*/, , obj->SLIP))
//             {
//                 result = 0;
//             }
//         }
//
//         // TODO create Msg
//         if(result)
//         {
//             uint8_t sequenceNo = Msg_ParamGetSequencenr(msg);
//             lastInSequence = Msg_ParamFcGetLastinsequence(msg);
//             result = (sequenceNo == EcpectedSequenceNo);
//             EcpectedSequenceNo++;
//         }
//         if(result)
//         {
//             result = /*Set all handlers*/;
//         }
//         if(result) // Set was successful, params are loaded from mem.
//         {
//
//         }
//         else // Read or set was not successful, send Nack.
//         {
//
//             lastInSequence = 1;
//         }
//     }
//     return result;
// }

