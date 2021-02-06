/*
 * AppLogSerialBackend.c
 *
 *  Created on: Aug 12, 2019
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

#include "Components/AppLog/inc/AppLogBackend.h"
#include "QuadFC/QuadFC_Serial.h"
#include "BoardConfig.h"
#define APP_LOG_INTERNAL_BUFFER (512)
#define APP_LOG_RECEIVE_BUFF (2)
struct AppLogBackend
{
    uint8_t write;
    uint8_t receiveBuff[APP_LOG_RECEIVE_BUFF];
};

AppLogBackend_t* AppLogBackend_CreateObj()
{
    AppLogBackend_t* obj = pvPortMalloc(sizeof(AppLogBackend_t));
    if(!obj)
    {
        return NULL;
    }
    QuadFC_SerialOptions_t opt = {
        115200,
        EightDataBits,
        NoParity,
        OneStopBit,
        NoFlowControl,
        obj->receiveBuff,
        APP_LOG_RECEIVE_BUFF
    };
    QuadFC_SerialInit(APPLOG_SERIAL_BUS, &opt);
    return obj;
}

void AppLogBackend_DeleteObj(AppLogBackend_t* obj)
{
    if(!obj)
    {
        return;
    }
    vPortFree(obj);
}

uint8_t AppLogBackend_Report(AppLogBackend_t* obj, uint8_t* buffer, uint32_t buffer_size)
{
    if(!obj || ! buffer || !buffer_size)
    {
        return 0;
    }
    uint8_t result = 0;
    QuadFC_Serial_t serialData;
    serialData.buffer = buffer;
    serialData.bufferLength = buffer_size;
    result = QuadFC_SerialWrite(&serialData, APPLOG_SERIAL_BUS, portMAX_DELAY);

    return result;
}

uint8_t AppLogBackend_GetLog(AppLogBackend_t* obj, uint8_t* buffer, uint32_t buffer_size)
{
    return 0;
}
