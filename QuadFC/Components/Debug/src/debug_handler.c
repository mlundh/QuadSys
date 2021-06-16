/*
* debug_handler.c
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
#include "Debug/inc/debug_handler.h"
#include "Messages/inc/Msg_TestTransmission.h"
#include "Messages/inc/Msg_Debug.h"
#include "Messages/inc/Msg_TestMem.h"
#include "Messages/inc/Msg_TestMemReg.h"

#include "Components/AppLog/inc/AppLog.h"
#include <string.h>
#include "Utilities/inc/run_time_stats.h"
#include "HAL/QuadFC/QuadFC_Memory.h"


struct debug_handler
{
   int tmp;
};

uint8_t Com_HandleDebug(eventHandler_t* obj, void* data, moduleMsg_t* msg);

uint8_t Com_TestTransmission(eventHandler_t* obj, void* data, moduleMsg_t* msg);

uint8_t Com_TestMemory(eventHandler_t* obj, void* data, moduleMsg_t* msg);

uint8_t Com_MemoryReadReg(eventHandler_t* obj, void* data, moduleMsg_t* msg);

debug_handler_t* Debug_CreateHandler(eventHandler_t* obj)
{
    debug_handler_t* handler = pvPortMalloc(sizeof(debug_handler_t));
    // Subscribe to the events that the task is interested in.
    Event_RegisterCallback(obj, Msg_TestTransmission_e, Com_TestTransmission, NULL);
    Event_RegisterCallback(obj, Msg_Debug_e, Com_HandleDebug, NULL);
    Event_RegisterCallback(obj, Msg_TestMem_e, Com_TestMemory, NULL);
    Event_RegisterCallback(obj, Msg_TestMemReg_e, Com_MemoryReadReg, NULL);

    return handler;
}

uint8_t Com_TestTransmission(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || data || !msg) // data should be null and not used.
    {
        return 0;
    }

    uint8_t result = 1;

    if(Msg_TestTransmissionGetTest(msg) != 0xDEADBEEF)
    {
        LOG_ENTRY(obj, "SerialTest: failed, did not get correct data!");
        result = 0;
    }
    if(strncmp((char*)Msg_TestTransmissionGetPayload(msg),"Test string\0",11) != 0) // Null termination not sent serially.
    {
        LOG_ENTRY(obj, "SerialTest: failed, expected \"%s\", got \"%s\"!","Test string\0", Msg_TestTransmissionGetPayload(msg));
        LOG_ENTRY(obj, "SerialTest: msgLength = %ld ", Msg_TestTransmissionGetPayloadlength(msg));
        result = 0;
    }
    if(result)
    {
        moduleMsg_t* reply = Msg_TestTransmissionCreate(Msg_GetSource(msg),0,1,10);
        Msg_SetRequireAck(reply, 0);
        strncpy((char*)Msg_TestTransmissionGetPayload(reply), "Test OK\0",8);
        Msg_TestTransmissionSetPayloadlength(reply, 8);
        LOG_ENTRY(obj, "SerialTest: Passed, sending reply.");
        Event_Send(obj, reply);
    }
    return result;
}


#define DBG_MSG_REPLY_LENGTH (255)

uint8_t Com_HandleDebug(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || data || !msg) // data should be null and not used.
    {
        return 0;
    }
    uint8_t result = 0;

    switch ( Msg_DebugGetControl(msg) )
    {
    case QSP_DebugGetRuntimeStats:
        {   
        moduleMsg_t* reply = Msg_DebugCreate(Msg_GetSource(msg), 0, QSP_DebugSetRuntimeStats, DBG_MSG_REPLY_LENGTH);

        QuadFC_RuntimeStats(Msg_DebugGetPayload(reply) , Msg_DebugGetPayloadbufferlength(reply));
        //vTaskList((char *)(QSP_GetPayloadPtr(obj->QspRespPacket)));
        uint16_t len =  strlen((char *)Msg_DebugGetPayload(reply));
        Msg_DebugSetPayloadlength(reply, len);
        Event_Send(obj, reply);
        result = 1;
        }
        break;
    default:
        break;
    }

    return result;
}

uint8_t Com_TestMemory(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || data || !msg) // data should be null and not used.
    {
        return 0;
    }

    uint8_t size = Msg_TestMemGetSize(msg);
    uint8_t startAddr = Msg_TestMemGetStartaddr(msg);
    uint8_t startNumber = Msg_TestMemGetStartnumber(msg);

    if(size > 40)
    {
        size = 40;
    }

    uint8_t endNumber = startNumber + size;
    if(endNumber < startNumber)
    {
        endNumber = 255;
        size = endNumber - startNumber;
    }

    uint8_t memData[size];

    uint8_t result = 0;
    switch (Msg_TestMemGetWrite(msg))
    {
    case 1: // write
        for (uint32_t i = startNumber; i < endNumber; i++)
        {
            memData[i - startNumber] = i;
        }
        result = Mem_Write(startAddr, size, memData, size);
        break;
    case 0: // read
        {
            for (uint32_t i = 0; i < size; i++)
            {
                memData[i] = 0;
            }
            result = Mem_Read(startAddr, size, memData, size);
            LOG_ENTRY(obj, "SpiTest: read:\n");
            char printBuffer [size*4];
            for(int i = 0; i < size; i++)
            {  
                char buf[4];
                sprintf(buf, "\n%d", memData[i]);
                strncat(printBuffer, buf, 4);
            }
            LOG_ENTRY(obj, "%s", printBuffer);
        }
        break;
    default:
        LOG_ENTRY(obj, "SpiTest: failed, read or write should be 1 or 0!");
        break;
    }
    return result;
}

uint8_t Com_MemoryReadReg(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
    if(!obj || data || !msg) // data should be null and not used.
    {
        return 0;
    }

    uint8_t readSr = Msg_TestMemRegGetRdsr(msg);
    uint8_t readId = Msg_TestMemRegGetDeviceid(msg);

    uint8_t result = 0;
   
    if(readId)
    {
        uint8_t txBuff[5] = {159, 0, 0, 0, 0};
        uint8_t rxBuff[5] = {0xff, 0xff, 0xff, 0xff, 0xff};
        Mem_ReadWriteRaw(rxBuff, txBuff, 5);
        LOG_ENTRY(obj, "SpiTest: ID = %u, %u, %u, %u", rxBuff[1], rxBuff[2], rxBuff[3], rxBuff[4]);

    }
    if(readSr)
    {
        uint8_t txBuff[2] = {5, 0};
        uint8_t rxBuff[2] = {0xff, 0xff};
        Mem_ReadWriteRaw(rxBuff, txBuff, 2);
        LOG_ENTRY(obj, "SpiTest: Sr = %u", rxBuff[1]);
    }

    return result;
}