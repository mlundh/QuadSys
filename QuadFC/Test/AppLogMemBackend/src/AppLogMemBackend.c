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
#include "FreeRTOS.h"
#include <string.h>
#define REPORT_LENGTH (500)

struct AppLogBackend
{
    uint8_t* report[REPORT_LENGTH];
};

AppLogBackend_t* AppLogBackend_CreateObj()
{
    AppLogBackend_t* obj = pvPortMalloc(sizeof(AppLogBackend_t));
    if(!obj)
    {
        return NULL;
    }
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
    if(buffer_size <= REPORT_LENGTH)
    {
        memcpy(&obj->report[0], buffer, buffer_size);
        return 1;
    }
    return 0;
}

uint8_t AppLogBackend_GetLog(AppLogBackend_t* obj, uint8_t* buffer, uint32_t buffer_size)
{
    if(buffer_size <= REPORT_LENGTH)
    {
        memcpy(buffer, &obj->report[0], buffer_size);
        return 1;
    }
    return 0;
}
