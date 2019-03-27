/*
 * Msg_Param.c
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

#ifndef MODULES_MESSAGES_INC_MSG_PARAM_H_
#define MODULES_MESSAGES_INC_MSG_PARAM_H_

#include "MsgBase/inc/message_base.h"
#include "MsgBase/inc/common_types.h"
#include "MsgBase/inc/msg_enums.h"
#include "MsgBase/inc/msgAddr.h"



moduleMsg_t* Msg_ParamCreate(uint32_t destination, uint8_t msgNr
    , uint8_t control, uint8_t sequenceNr, uint8_t lastInSequence, uint8_t* payload, uint32_t Payloadlength, uint32_t Payloadbufferlength);

uint8_t Msg_ParamGetControl(moduleMsg_t* msg);

void Msg_ParamSetControl(moduleMsg_t* msg, uint8_t control);

uint8_t Msg_ParamGetSequencenr(moduleMsg_t* msg);

void Msg_ParamSetSequencenr(moduleMsg_t* msg, uint8_t sequenceNr);

uint8_t Msg_ParamGetLastinsequence(moduleMsg_t* msg);

void Msg_ParamSetLastinsequence(moduleMsg_t* msg, uint8_t lastInSequence);

uint8_t* Msg_ParamGetPayload(moduleMsg_t* msg);

void Msg_ParamSetPayload(moduleMsg_t* msg, uint8_t* payload);

uint32_t Msg_ParamGetPayloadlength(moduleMsg_t* msg);

void Msg_ParamSetPayloadlength(moduleMsg_t* msg, uint32_t Payloadlength);

uint32_t Msg_ParamGetPayloadbufferlength(moduleMsg_t* msg);

void Msg_ParamSetPayloadbufferlength(moduleMsg_t* msg, uint32_t Payloadbufferlength);


uint8_t* Msg_ParamSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size);

uint8_t* Msg_ParamDeserialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size);

#endif /* MODULES_MESSAGES_INC_MSG_PARAM_H_ */

