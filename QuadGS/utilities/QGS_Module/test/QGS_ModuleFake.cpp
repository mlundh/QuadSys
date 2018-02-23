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

namespace QuadGS {

QGS_ModuleFake::QGS_ModuleFake(std::string name)
:QGS_MessageHandlerBase(name), mNrMsg(0),mReturnNxtMsg(false)
{
}

QGS_ModuleFake::~QGS_ModuleFake()
{

}


int QGS_ModuleFake::getNrMsg()
{
	return mNrMsg.load();
}

void QGS_ModuleFake::sendDummyMsg(messageTypes_t type)
{
	QGS_ModuleMsg::ptr msg = QGS_ModuleMsg::Create(type);
	sendMsg(std::move(msg));
}

void QGS_ModuleFake::returnNxtMsg(bool flag)
{
	mReturnNxtMsg.store(flag);
}

void QGS_ModuleFake::registerCommands()
{
}

void QGS_ModuleFake::subscribeMsg(messageTypes_t type)
{
	QGS_Module::subscribeMsg(type);
}

void QGS_ModuleFake::process(QGS_ModuleMsg* message)
{
	mNrMsg++;
	if(mReturnNxtMsg.load())
	{
		QGS_ModuleMsg::ptr ptr = QGS_ModuleMsg::Create(*message);
		sendMsg(std::move(ptr));
		mReturnNxtMsg.store(false);
	}
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

void QGS_ThreadedModuleFake::sendDummyMsg(messageTypes_t type)
{
	QGS_ModuleMsg::ptr msg = QGS_ModuleMsg::Create(type);
	sendMsg(std::move(msg));
}

void QGS_ThreadedModuleFake::returnNxtMsg(bool flag)
{
	mReturnNxtMsg.store(flag);
}

void QGS_ThreadedModuleFake::registerCommands()
{
}

void QGS_ThreadedModuleFake::subscribeMsg(messageTypes_t type)
{
	QGS_Module::subscribeMsg(type);
}

void QGS_ThreadedModuleFake::process(QGS_ModuleMsg* message)
{
	mNrMsg++;
	if(mReturnNxtMsg.load())
	{
		QGS_ModuleMsg::ptr ptr = QGS_ModuleMsg::Create(*message);
		sendMsg(std::move(ptr));
		mReturnNxtMsg.store(false);
	}
}

void QGS_ThreadedModuleFake::module()
{
	// do work...
	usleep(500);

	// process messages, non blocking
	handleMessages(false);
}


} /* namespace QuadGS */
