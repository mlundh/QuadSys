/*
 * QGSModule.cpp
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


#include "QGS_Module.h"
#include "QGS_Router.h"
#include "Msg_Stop.h"

namespace QuadGS
{

QGS_Module::UiCommand::UiCommand(std::string command,  std::string doc, UiFcn function):
				command(command), doc(doc), function(function)
{

}

QGS_Module::QGS_Module()
:QGS_MessageHandlerBase(msgAddr_t::Unassigned), mSendFcn(NULL)
{

}

QGS_Module::~QGS_Module()
{

}
void QGS_Module::bind(QGS_Router* router)
{
	router->bind(this);
}


void QGS_Module::setReceivingFcn(receivingFcn_t fcn, msgAddr_t address)
{
	if(fcn)
	{
		mReceiveFcns[address] = fcn;
	}
}

std::map<msgAddr_t, WriteFcn>& QGS_Module::getReceivingFcns()
{
	if(!mReceiveFcns.empty())
	{
		return mReceiveFcns;
	}
	else
	{
		throw std::runtime_error("No message receiving function set. Set the function before binding.");
	}
}

void QGS_Module::sendMsg(std::unique_ptr<QGS_ModuleMsgBase> message)
{
	if(mSendFcn)
	{
		message->setSource(getName());
		mSendFcn(std::move(message));
	}
	else
	{
		throw std::runtime_error("No send function set. Bind before sending messages.");
	}
}

void QGS_Module::sendExternalMsg(std::unique_ptr<QGS_ModuleMsgBase> message)
{
	if(mSendFcn)
	{
		mSendFcn(std::move(message));
	}
	else
	{
		throw std::runtime_error("No send function set. Bind before sending messages.");
	}
}


void QGS_Module::setSendFunc(WriteFcn func)
{
	if(func)
	{
		mSendFcn = func;
	}
}

QGS_ReactiveModule::QGS_ReactiveModule()
:QGS_MessageHandlerBase(msgAddr_t::Unassigned)
{
	setReceivingFcn(std::bind(&QGS_ReactiveModule::ReceivingFcn, this, std::placeholders::_1), getName());
}

QGS_ReactiveModule::~QGS_ReactiveModule()
{

}


void QGS_ReactiveModule::ReceivingFcn(std::unique_ptr<QGS_ModuleMsgBase> message)
{
	message->dispatch(this);
}


QGS_ThreadedModule::QGS_ThreadedModule()
:QGS_MessageHandlerBase(msgAddr_t::Unassigned), mStop(false)
{
	setReceivingFcn(std::bind(&QGS_ThreadedModule::ReceivingFcn, this, std::placeholders::_1), getName());
}

QGS_ThreadedModule::~QGS_ThreadedModule()
{
	stopProcessing();
}

void QGS_ThreadedModule::startProcessing()
{
	if(!mThread.joinable())
	{
		mThread = std::thread(std::bind(&QGS_ThreadedModule::runThread, this));
	}
}

void QGS_ThreadedModule::stopProcessing()
{
	QGS_ModuleMsgBase::ptr ptr  = std::make_unique<Msg_Stop>(getName());
	mFifo.push(std::move(ptr)); // send stop to own fifo.

	if(mThread.joinable())
	{
		mThread.join();
	}
}

void QGS_ThreadedModule::setProcessingFcn(processingFcn fcn)
{
	if(fcn)
	{
		mProcessingFcn = fcn;
	}
	else
	{
		mLogger.QuadLog(severity_level::warning, "No processing function set.");
	}
}

void QGS_ThreadedModule::handleMessages(bool blocking)
{
	if(!blocking)
	{
		if(mFifo.empty()) // we are the only ones poping from the queue, if it is non-empty, then we will not block.
		{
			return; // empty fifo - nothing to process, return!
		}
	}

	QGS_ModuleMsgBase::ptr msg = mFifo.dequeue();
	if(msg->getType() == messageTypes_t::Msg_Stop_e)
	{
		mStop = true;
	}
	else
	{
		msg->dispatch(this);
	}
}

void QGS_ThreadedModule::waitForFinish()
{
	mThread.join();

}
void QGS_ThreadedModule::ReceivingFcn(std::unique_ptr<QGS_ModuleMsgBase> message)
{
	mFifo.push(std::move(message));
}

void QGS_ThreadedModule::runThread()
{
	while(!mStop)
	{
		if(mProcessingFcn)
		{
			mProcessingFcn();
		}
		else
		{
			mLogger.QuadLog(severity_level::warning, "No processing function set.");
		}
	}
	mLogger.QuadLog(severity_level::info, "Stopping threaded module.");
}

} /* namespace QuadGS */


