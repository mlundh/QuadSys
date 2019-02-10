/*
 * message_base.h
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


#ifndef MODULES_MSGBASE_INC_MESSAGE_BASE_H_
#define MODULES_MSGBASE_INC_MESSAGE_BASE_H_

#include <stdint.h>
#include "messageTypes.h"
#include "msg_enums.h"
#include "Utilities/inc/serialization.h"

/**
 * Struct describing a message. This is the structure that will be
 * sent over the internal queue. The data field should be used with caution,
 * nothing in the event system guarantees that it is thread safe. So use for sending
 * structures that are only used by one thread at the time, or that them self are thread
 * safe, this could be a new queue that contains the actual data.
 */

typedef struct
{
    uint32_t            type;
    uint32_t            mSource;
    uint32_t            mDestination;
    uint8_t             mMsgNr;
    uint32_t            mAllocatedSize;
}moduleMsg_t;

moduleMsg_t* Msg_Create(uint32_t type, uint32_t source, uint32_t destination, uint8_t msgNr);

uint8_t Msg_Delete(moduleMsg_t* msg);

uint32_t Msg_GetType(moduleMsg_t* msg);

uint32_t Msg_GetSource(moduleMsg_t* msg);
void Msg_SetSource(moduleMsg_t* msg, uint32_t source);

uint32_t Msg_GetDestination(moduleMsg_t* msg);
void Msg_SetDestination(moduleMsg_t* msg, uint32_t desitnation);

uint32_t Msg_GetMsgNr(moduleMsg_t* msg);
void Msg_SetMsgNr(moduleMsg_t* msg, uint32_t msgNr);

uint8_t* Msg_Serialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t* buffer_size);
uint8_t* Msg_DeSerialize(moduleMsg_t* msg, uint8_t* buffer, uint32_t* buffer_size);

moduleMsg_t* Msg_Clone(moduleMsg_t* msg);

#endif /* MODULES_MSGBASE_INC_MESSAGE_BASE_H_ */
