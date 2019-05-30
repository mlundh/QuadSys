/*
 * Msg_ParamFc.c
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

#ifndef MODULES_MESSAGES_INC_MSG_PARAMFC_H_
#define MODULES_MESSAGES_INC_MSG_PARAMFC_H_

#include "MsgBase/inc/message_base.h"
#include "MsgBase/inc/common_types.h"
#include "MsgBase/inc/msg_enums.h"
#include "MsgBase/inc/msgAddr.h"



moduleMsg_t* Msg_ParamFcCreate(uint32_t destination, uint8_t msgNr
    , uint8_t control, int8_t sequenceNr, uint8_t lastInSequence, uint32_t Payloadbufferlength);

uint8_t Msg_ParamFcGetControl(moduleMsg_t* msg);

void Msg_ParamFcSetControl(moduleMsg_t* msg, uint8_t control);

int8_t Msg_ParamFcGetSequencenr(moduleMsg_t* msg);

void Msg_ParamFcSetSequencenr(moduleMsg_t* msg, int8_t sequenceNr);

uint8_t Msg_ParamFcGetLastinsequence(moduleMsg_t* msg);

void Msg_ParamFcSetLastinsequence(moduleMsg_t* msg, uint8_t lastInSequence);

uint8_t* Msg_ParamFcGetPayload(moduleMsg_t* msg);

void Msg_ParamFcSetPayload(moduleMsg_t* msg, uint8_t* payload);

uint32_t Msg_ParamFcGetPayloadlength(moduleMsg_t* msg);

void Msg_ParamFcSetPayloadlength(moduleMsg_t* msg, uint32_t Payloadlength);

uint32_t Msg_ParamFcGetPayloadbufferlength(moduleMsg_t* msg);

void Msg_ParamFcSetPayloadbufferlength(moduleMsg_t* msg, uint32_t Payloadbufferlength);


uint8_t* Msg_ParamFcSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size);

uint8_t* Msg_ParamFcDeserialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size);

#endif /* MODULES_MESSAGES_INC_MSG_PARAMFC_H_ */

