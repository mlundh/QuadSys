/*
 * Msg_CtrlModeReq.c
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

#ifndef MODULES_MESSAGES_INC_MSG_CTRLMODEREQ_H_
#define MODULES_MESSAGES_INC_MSG_CTRLMODEREQ_H_

#include "MsgBase/inc/message_base.h"
#include "MsgBase/inc/common_types.h"
#include "MsgBase/inc/msg_enums.h"
#include "MsgBase/inc/msgAddr.h"



moduleMsg_t* Msg_CtrlModeReqCreate(uint32_t destination, uint8_t msgNr
    , uint8_t mode);

uint8_t Msg_CtrlModeReqGetMode(moduleMsg_t* msg);

void Msg_CtrlModeReqSetMode(moduleMsg_t* msg, uint8_t mode);


uint8_t* Msg_CtrlModeReqSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size);

uint8_t* Msg_CtrlModeReqDeserialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size);

#endif /* MODULES_MESSAGES_INC_MSG_CTRLMODEREQ_H_ */

