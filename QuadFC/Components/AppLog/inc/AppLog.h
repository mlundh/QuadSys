/*
 * AppLog.h
 *
 *  Created on: nov 19, 2019
 *      Author: mlundh
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
#include <string.h>
#include "Messages/inc/Msg_AppLog.h"
#include "FreeRTOS.h"

#define LOG_LENGTH_MAX (200)
#define LOG_ENTRY(addr, evHandler,  format, ...) {  \
    moduleMsg_t* msgAppLog = Msg_AppLogCreate(addr,0,writeAppLog,LOG_LENGTH_MAX); \
    snprintf((char*)Msg_AppLogGetPayload(msgAppLog), LOG_LENGTH_MAX, "%8lu: "format"\n" ,xTaskGetTickCount(), ##__VA_ARGS__);\
    uint32_t buffSize = strlen((char*)Msg_AppLogGetPayload(msgAppLog))+1;\
    Msg_AppLogSetPayloadlength(msgAppLog, buffSize);\
    Event_Send(evHandler, msgAppLog);\
    }
