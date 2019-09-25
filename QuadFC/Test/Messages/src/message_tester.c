/*
 * message_tester.c
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


#include "../message_tester.h"
#include "Messages/inc/Msg_Param.h"
#include <string.h>

uint8_t MessageT_TestSerializationMsg(TestFw_t* Tobj);
uint8_t MessageT_TestSerializationParam(TestFw_t* Tobj);

uint8_t msgCmp(moduleMsg_t* msg, moduleMsg_t* msg2)
{
    uint8_t result = 0;
    if((msg->mDestination == msg2->mDestination) &&
        (msg->mMsgNr == msg2->mMsgNr) &&
        (msg->mSource == msg2->mSource) &&
        (msg->type == msg2->type))
    {
        result = 1;
    }
    return result;
}

void MessageT_GetTCs(TestFw_t* Tobj)
{
    TestFW_RegisterTest(Tobj, "Msg Test serialization", MessageT_TestSerializationMsg);
    TestFW_RegisterTest(Tobj, "Msg Test Param serialization", MessageT_TestSerializationParam);
}



#define PAYLOAD_LENGTH (50)

uint8_t MessageT_TestSerializationMsg(TestFw_t* Tobj)
{
    moduleMsg_t* msg = Msg_Create(5,88);
    Msg_SetSource(msg, 4);
    Msg_SetMsgNr(msg, 99);


    uint8_t buffer[PAYLOAD_LENGTH] = {0};
    uint8_t* buffP = &buffer[0];

    {
        Msg_Serialize(msg, buffP, PAYLOAD_LENGTH);
    }
    moduleMsg_t* msgVerify = Msg_Create(0,0);
    {
        Msg_DeSerialize(msgVerify, buffP, PAYLOAD_LENGTH);
    }
    uint8_t result = 0;
    if(msgCmp(msg, msgVerify))
    {
        result = 1;
    }

    Msg_Delete(&msg);
    Msg_Delete(&msgVerify);

    return result;
}


uint8_t MessageT_TestSerializationParam(TestFw_t* Tobj)
{
    moduleMsg_t* msg = Msg_ParamCreate(1,2,3,4,5,PAYLOAD_LENGTH);

    uint8_t payload[] = {"Testing a cool thing right now.\0"};

    uint32_t payloadLength = strlen((char*)payload);

    Msg_ParamSetPayloadlength(msg, payloadLength);

    uint8_t* pl = Msg_ParamGetPayload(msg);
    memcpy(pl, payload, payloadLength);


    uint8_t buffer[PAYLOAD_LENGTH * 2] = {0};
    uint8_t* buffP = &buffer[0];

    Msg_ParamSerialize(msg, buffP, PAYLOAD_LENGTH * 2);

    moduleMsg_t* msgVerify  = Msg_ParamDeserialize(buffP, PAYLOAD_LENGTH * 2);


    uint8_t result = 0;
    if(msgCmp(msg, msgVerify) && memcmp(Msg_ParamGetPayload(msg), Msg_ParamGetPayload(msgVerify), payloadLength) == 0)
    {
        result = 1;
    }

    Msg_Delete(&msg);
    Msg_Delete(&msgVerify);

    return result;
}
