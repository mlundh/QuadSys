/*
 * QGS_ModuleFake.cpp
 *
 *  Copyright (C) 2018 Martin Lundh
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

#include "QGS_ModuleFake.h"
#include "QGS_ModuleIoMsg.h"
#include "QGS_ParamMsg.h"
#include "QGS_DebugMsg.h"

namespace QuadGS {


QGS_ModuleFake::QGS_ModuleFake(std::string name)
:QGS_MessageHandlerBase(name), mNrMsg(0),mReturnNxtMsg(false)
{
	mCommands.push_back(QGS_UiCommand_("TestFcn", std::bind(&QGS_ModuleFake::testCommand, this, std::placeholders::_1), "test command."));
}

QGS_ModuleFake::~QGS_ModuleFake()
{

}


int QGS_ModuleFake::getNrMsg()
{
	return mNrMsg.load();
}

void QGS_ModuleFake::sendDummyDebugIoMsg()
{
	QGS_DebugMsg::ptr dbgMsg = std::make_unique<QGS_DebugMsg>(0,"NoDebugData");
	QGS_ModuleMsgBase::ptr msg = std::make_unique<QGS_ModuleIoMsg<QGS_DebugMsg>>(std::move(dbgMsg), msgDebug);
	sendMsg(std::move(msg));
}


void QGS_ModuleFake::sendDummyParamIoMsg()
{
	QGSParamMsg::ptr paramMsg = std::make_unique<QGSParamMsg>(0, "NoValidData", 0, 1);
	QGS_ModuleMsgBase::ptr msg = std::make_unique<QGS_ModuleIoMsg<QGSParamMsg>>(std::move(paramMsg), msgParam);
	sendMsg(std::move(msg));
}

void QGS_ModuleFake::returnNxtMsg(bool flag)
{
	mReturnNxtMsg++;
}

std::string QGS_ModuleFake::testCommand(std::string args)
{
	if(args.compare("args") == 0)
	{
		return "OK";
	}
	return "NOK";
}


void QGS_ModuleFake::subscribeMsg(messageTypes_t type)
{
	QGS_Module::subscribeMsg(type);
}

void QGS_ModuleFake::process_internal(QGS_ModuleMsgBase* bptr)
{
	mNrMsg++;
	if(mReturnNxtMsg.load() > 0)
	{
		QGS_ModuleMsgBase::ptr ptr = std::unique_ptr<QGS_ModuleMsgBase>(bptr->clone());
		sendMsg(std::move(ptr));
		mReturnNxtMsg--;
	}
}
void QGS_ModuleFake::process(QGS_CommandRsltMsg* message)
{
	process_internal(message);
}

void QGS_ModuleFake::process(QGS_ModuleIoMsg<QGS_DebugMsg>* message)
{
	process_internal(message);
}
void QGS_ModuleFake::process(QGS_ModuleIoMsg<QGSParamMsg>* message)
{
	process_internal(message);
}




QGS_ThreadedModuleFake::QGS_ThreadedModuleFake(std::string name)
:QGS_MessageHandlerBase(name), mNrMsg(0),mReturnNxtMsg(false)
{
	setProcessingFcn(std::bind(&QGS_ThreadedModuleFake::module, this));
	startProcessing();
}

QGS_ThreadedModuleFake::~QGS_ThreadedModuleFake()
{
	stopProcessing();

}


int QGS_ThreadedModuleFake::getNrMsg()
{
	return mNrMsg.load();
}


void QGS_ThreadedModuleFake::returnNxtMsg(bool flag)
{
	mReturnNxtMsg.store(flag);
}


void QGS_ThreadedModuleFake::subscribeMsg(messageTypes_t type)
{
	QGS_Module::subscribeMsg(type);
}

void QGS_ThreadedModuleFake::sendDummyDebugMsg()
{
	QGS_DebugMsg::ptr dbgMsg = std::make_unique<QGS_DebugMsg>(0,"NoDebugData");
	QGS_ModuleMsgBase::ptr msg = std::make_unique<QGS_ModuleIoMsg<QGS_DebugMsg>>(std::move(dbgMsg), msgDebug);
	sendMsg(std::move(msg));
}


void QGS_ThreadedModuleFake::sendDummyParamMsg()
{
	QGSParamMsg::ptr paramMsg = std::make_unique<QGSParamMsg>(0, "NoValidData", 0, 1);
	QGS_ModuleMsgBase::ptr msg = std::make_unique<QGS_ModuleIoMsg<QGSParamMsg>>(std::move(paramMsg), msgParam);
	sendMsg(std::move(msg));
}


void QGS_ThreadedModuleFake::process_internal(QGS_ModuleMsgBase* bptr)
{
	mNrMsg++;
	if(mReturnNxtMsg.load() > 0)
	{
		QGS_ModuleMsgBase::ptr ptr = std::unique_ptr<QGS_ModuleMsgBase>(bptr->clone());
		sendMsg(std::move(ptr));
		mReturnNxtMsg--;
	}
}
void QGS_ThreadedModuleFake::process(QGS_CommandRsltMsg* message)
{
	process_internal(message);
}

void QGS_ThreadedModuleFake::process(QGS_ModuleIoMsg<QGS_DebugMsg>* message)
{
	process_internal(message);
}
void QGS_ThreadedModuleFake::process(QGS_ModuleIoMsg<QGSParamMsg>* message)
{
	process_internal(message);
}


void QGS_ThreadedModuleFake::module()
{
	// do work...
	usleep(500);

	// process messages, non blocking
	handleMessages(false);
}





QGS_IoModuleFake::QGS_IoModuleFake(std::string name)
:QGS_MessageHandlerBase(name), mNrMsg(0)
{
	setProcessingFcn(std::bind(&QGS_IoModuleFake::module, this));
	startProcessing();

}

QGS_IoModuleFake::~QGS_IoModuleFake()
{
	stopProcessing();

}

void QGS_IoModuleFake::getCommands()
{
	QGS_CommandReqMsg::ptr msg = std::make_unique<QGS_CommandReqMsg>();
	sendMsg(std::move(msg));
}

void QGS_IoModuleFake::subscribeMsg(messageTypes_t type)
{
	QGS_Module::subscribeMsg(type);
}

void QGS_IoModuleFake::sendCommandMsg()
{
	QGS_CommandMsg::ptr msg = std::make_unique<QGS_CommandMsg>("TestFcn", "args");
	sendMsg(std::move(msg));
	return;
}


void QGS_IoModuleFake::process(QGS_CommandRsltMsg* message)
{
	mResponce.push_back(message->mResult);
}

void QGS_IoModuleFake::process(QGS_CommandReqRspMsg* message)
{
	for(auto i : message->mCommands)
	{
		mCommands.push_back(i);
	}
}


void QGS_IoModuleFake::module()
{
	// do work...
	usleep(5);

	// process messages, non blocking
	handleMessages(false);
}


} /* namespace QuadGS */
