/*
 * Msg_RegisterHandler.c
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

#ifndef MODULES_MESSAGES_INC_MSG_REGISTERHANDLER_H_
#define MODULES_MESSAGES_INC_MSG_REGISTERHANDLER_H_

#include "MsgBase/inc/message_base.h"
#include "Messages/inc/common_types.h"
#include "Messages/inc/msg_enums.h"
#include "Messages/inc/msgAddr.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"


#define Msg_RegisterHandlerCreate(destination, msgNr , data) Msg_RegisterHandlerCreatePool(NULL, destination, msgNr , data)

moduleMsg_t* Msg_RegisterHandlerCreatePool(messagePool_t* pool, uint32_t destination, uint8_t msgNr
    , QueueHandle_t data);

QueueHandle_t Msg_RegisterHandlerGetData(moduleMsg_t* msg);

void Msg_RegisterHandlerSetData(moduleMsg_t* msg, QueueHandle_t data);


uint8_t* Msg_RegisterHandlerSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t buffer_size);

moduleMsg_t* Msg_RegisterHandlerDeserialize(uint8_t* buffer, uint32_t buffer_size);

uint32_t Msg_RegisterHandlerGetMessageSize();


#endif /* MODULES_MESSAGES_INC_MSG_REGISTERHANDLER_H_ */

