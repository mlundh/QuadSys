/*
 * AppLogTest.c
 *
 *  Created on: Aug 15, 2019
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

#include "../AppLogTest.h"
#include "Components/AppLog/src/AppLogHandlerPrivate.h"

#include "Components/AppLog/inc/AppLogHandler.h"

#include <stddef.h>
void AppLog_GetTCs(TestFw_t* obj)
{
    TestFW_RegisterTest(obj, "AppLog Create", AppLogHandler_TestCreate);

}
#define SHORT_MSG (200)
uint8_t AppLogHandler_TestCreate(TestFw_t* obj)
{
    AppLogHandler_t* logobj = AppLogHandler_Create(NULL);

    uint8_t result = 1;
    if(!logobj)
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"Could not create a AppLogHandler object.\n");
        TestFW_Report(obj, tmpstr);
        result = 0;
    }
    if(!result ||
            logobj->evHandler || //Initialized to null...
            !logobj->backend )
    {
        char tmpstr[SHORT_MSG] = {0};
        snprintf (tmpstr, SHORT_MSG,"AppLogHandler object not correctly initiated.\n");
        TestFW_Report(obj, tmpstr);
        result = 0;
    }
    return 0;
}

