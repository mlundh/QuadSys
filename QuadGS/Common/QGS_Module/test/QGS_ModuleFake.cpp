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
#include "Msg_ParamIo.h"
#include "Msg_DebugIo.h"
#include "Msg_Param.h"
namespace QuadGS {


FakeModule::FakeModule(msgAddr_t name)
:QGS_MessageHandlerBase(name), mNrMsg(0),mReturnNxtMsg(false)
{

}

FakeModule::~FakeModule()
{

}


int FakeModule::getNrMsg()
{
	return mNrMsg.load();
}

void FakeModule::sendDummyDebugMsg(msgAddr_t dest)
{
	QGS_DebugMsg::ptr dbgMsg = std::make_unique<QGS_DebugMsg>(0,"NoDebugData");
	QGS_ModuleMsgBase::ptr msg = std::make_unique<Msg_DebugIo>(dest, std::move(dbgMsg));
	sendMsg(std::move(msg));
}


void FakeModule::sendDummyParamMsg(msgAddr_t dest)
{
	QGSParamMsg::ptr paramMsg = std::make_unique<QGSParamMsg>(0, "NoValidData", 0, 1);
	QGS_ModuleMsgBase::ptr msg = std::make_unique<Msg_ParamIo>(dest, std::move(paramMsg));
	sendMsg(std::move(msg));
}

void FakeModule::returnNxtMsg(bool flag)
{
	mReturnNxtMsg++;
}

void FakeModule::process_internal(QGS_ModuleMsgBase* bptr)
{
	mNrMsg++;
	if(mReturnNxtMsg.load() > 0)
	{
		QGS_ModuleMsgBase::ptr ptr = std::unique_ptr<QGS_ModuleMsgBase>(bptr->clone());
		ptr->setDestination(ptr->getSource());
		sendMsg(std::move(ptr));
		mReturnNxtMsg--;
	}
}

void FakeModule::process(Msg_ParamIo* message)
{
	process_internal(message);
}
void FakeModule::process(Msg_DebugIo* message)
{
	process_internal(message);
}




QGS_ThreadedModuleFake::QGS_ThreadedModuleFake(msgAddr_t name)
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


void QGS_ThreadedModuleFake::sendDummyDebugMsg(msgAddr_t dest)
{
	QGS_DebugMsg::ptr dbgMsg = std::make_unique<QGS_DebugMsg>(0,"NoDebugData");
	QGS_ModuleMsgBase::ptr msg = std::make_unique<Msg_DebugIo>(dest, std::move(dbgMsg));
	sendMsg(std::move(msg));
}


void QGS_ThreadedModuleFake::sendDummyParamMsg(msgAddr_t dest)
{
	QGSParamMsg::ptr paramMsg = std::make_unique<QGSParamMsg>(0, "NoValidData", 0, 1);
	QGS_ModuleMsgBase::ptr msg = std::make_unique<Msg_ParamIo>(dest,std::move(paramMsg));
	sendMsg(std::move(msg));
}


void QGS_ThreadedModuleFake::process_internal(QGS_ModuleMsgBase* bptr)
{
	mNrMsg++;
	if(mReturnNxtMsg.load() > 0)
	{
		QGS_ModuleMsgBase::ptr ptr = std::unique_ptr<QGS_ModuleMsgBase>(bptr->clone());
		ptr->setDestination(ptr->getSource());
		sendMsg(std::move(ptr));
		mReturnNxtMsg--;
	}
}

void QGS_ThreadedModuleFake::process(Msg_DebugIo* message)
{
	process_internal(message);
}
void QGS_ThreadedModuleFake::process(Msg_ParamIo* message)
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


} /* namespace QuadGS */
