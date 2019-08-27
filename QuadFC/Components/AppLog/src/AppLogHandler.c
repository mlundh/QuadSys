/*
 * AppLogHandler.c
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
#include "AppLogHandlerPrivate.h"
#include "../inc/AppLogHandler.h"
#include "../inc/AppLogBackend.h"
#include "Messages/inc/Msg_AppLog.h"
#include "Messages/inc/msg_enums.h"

#define APPLOG_Length_MAX (255)

uint8_t AppLogHandler_HandleLog(eventHandler_t* obj, void* data, moduleMsg_t* msg);

AppLogHandler_t* AppLogHandler_Create(eventHandler_t* evHandler)
{
	AppLogHandler_t *obj = pvPortMalloc( sizeof(AppLogHandler_t) );
	if(!obj)
	{
		return NULL;
	}
	obj->evHandler = evHandler;
	obj->backend = AppLogBackend_CreateObj();

	Event_RegisterCallback(evHandler, Msg_AppLog_e, AppLogHandler_HandleLog, obj);

	return obj;
}


uint8_t AppLogHandler_HandleLog(eventHandler_t* obj, void* data, moduleMsg_t* msg)
{
	if(!obj || ! data || !msg || msg->type != Msg_AppLog_e)
	{
		return 0;
	}
	uint8_t result = 0;
	AppLogHandler_t* appLogObj = (AppLogHandler_t*)data; // The data should always be the handler object when a Msg_param is received.

	uint8_t control = Msg_AppLogGetControl(msg);

	switch (control)
	{
	case writeAppLog:
	{
		AppLogBackend_Report(appLogObj->backend, Msg_AppLogGetPayload(msg), Msg_AppLogGetPayloadlength(msg));
	}
	break;
	case getAppLog:
	{
		moduleMsg_t* msgRsp = Msg_AppLogCreate(Msg_GetSource(msg),0, writeAppLog, APPLOG_Length_MAX);
		AppLogBackend_GetLog(appLogObj->backend, Msg_AppLogGetPayload(msgRsp), Msg_AppLogGetPayloadbufferlength(msgRsp));
		break;
	}
	default:
	{

	}
	}
	return result;
}
