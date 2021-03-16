/*
 * QGSRouter.cpp
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

#include "QGS_Router.h"
#include "ThreadSafeFifo.hpp"
#include "QGS_Msg.h"
#include "QGS_Module.h"
#include "Msg_Stop.h"
#include "Msg_RegisterName.h"

namespace QuadGS
{

QGS_Router::QGS_Router(msgAddr_t name): 
	mFifo(200)
	, mPortWriteFcn(NULL)
	, mNrModules(0)
	, mName(name)
	, mStop(false)
	, mLog(msgAddrStr.at(name), std::bind(&QGS_Router::sendMsg, this, std::placeholders::_1))
{
	mThread = std::thread(std::bind(&QGS_Router::runRouter, this));
}

QGS_Router::~QGS_Router()
{
	QGS_ModuleMsgBase::ptr ptr  = std::make_unique<Msg_Stop>(mName);
	mFifo.push(std::move(ptr)); // send stop to own fifo.

	if(mThread.joinable())
	{
		mThread.join();
	}
}

void QGS_Router::bind(QGS_Module* module)
{
	module->setSendFunc(std::bind(&QGS_Router::incomingPort, this, std::placeholders::_1));
	std::map<msgAddr_t, WriteFcn>  map = module->getReceivingFcns();
	for(auto const& item : map)
	{
		checkUniqueName(item.first);
		mWriteFunctions[item.first] = item.second;
	}

}

void QGS_Router::bind(QGS_PortModule* module)
{
	bind(static_cast<QGS_Module*>(module)); // first bind normally.
	mPortWriteFcn = module->getPortFcn();
}


bool QGS_Router::done()
{
	return mFifo.empty();
}


void QGS_Router::incomingPort(QGS_ModuleMsgBase::ptr message)
{
	mFifo.push(std::move(message));
}


void QGS_Router::sendMsg(QGS_ModuleMsgBase::ptr message)
{
	// If we already know where to send to, then send.
	msgAddr_t dest = message->getDestination();
	if((dest & REGION_MASK) ==  msgAddrDomain_t::GS_e) // Did we get a local destination?
	{
		if(dest == msgAddr_t::GS_Broadcast_e) // Did we get a local broadcast?
		{
			internalBC(std::move(message));
		}
		else // No broadcast, only send to one!
		{
			internalSend(std::move(message), dest, false);
		}
	}
	else if((dest & REGION_MASK) ==  msgAddrDomain_t::BC_e) // did we get a global desitnation?
	{
		if(dest == msgAddr_t::Broadcast_e)
		{
			QGS_ModuleMsgBase::ptr ptr(message->clone());
			internalBC(std::move(message));
			externalSend(std::move(ptr));
		}
		else
		{
			mLog << "Received a message without destination!" ;
			mLog.flush();
		}

	}
	else // not a message to us, send to other side!
	{
		externalSend(std::move(message));
	}
}

void QGS_Router::externalSend(QGS_ModuleMsgBase::ptr message)
{
	if(mPortWriteFcn)
	{
		mPortWriteFcn(std::move(message));
	}
}


void QGS_Router::internalBC(QGS_ModuleMsgBase::ptr message)
{
	msgAddr_t source = message->getSource();
	for(auto module : mWriteFunctions)
	{
		if(module.first != source && message) // Do not return to sender
		{
			message->setDestination(module.first);
			message = internalSend(std::move(message), module.first, true);
		}
	}
}


QGS_ModuleMsgBase::ptr QGS_Router::internalSend(QGS_ModuleMsgBase::ptr message, msgAddr_t port, bool broadcast)
{
	//Make sure we have a write function.
	std::map<msgAddr_t, WriteFcn>::iterator it = mWriteFunctions.find(port);
	if(it != mWriteFunctions.end())
	{
		try
		{
			if(broadcast)
			{
				// each module gets its own copy. This ensures thread safety.
				QGS_ModuleMsgBase::ptr ptr(message->clone());
				mWriteFunctions[port](std::move(ptr));
			}
			else
			{
				// only one destination, move!
				mWriteFunctions[port](std::move(message));
			}
		}
		catch (std::runtime_error& e)
		{
			mLog.setMsgLogLevel(log_level::error);
			mLog << "Module: " << msgAddrStr.at(port) << " encountered an error: " << e.what();
			mLog.flush();
		}
	}
	else
	{
		mLog.setMsgLogLevel(log_level::error);
		mLog << "No port: " << msgAddrStr.at(port) << ", error in topology? ";
		mLog.flush();
	}
	return message;
}


void QGS_Router::runRouter()
{
	while(!mStop)
	{

		QGS_ModuleMsgBase::ptr msg(mFifo.dequeue());
		route(std::move(msg));
	}
	mLog.setMsgLogLevel(log_level::info);
	mLog << "Stopping router.";
	mLog.flush();
}


void QGS_Router::checkUniqueName(msgAddr_t name)
{
	std::map<msgAddr_t,WriteFcn>::iterator it = mWriteFunctions.find(name);
	if(it != mWriteFunctions.end())
	{
		mLog.setMsgLogLevel(log_level::warning);
		mLog << "Name collision, two modules named: " << msgAddrStr.at(name) << ". Advice to change name.";
		mLog.flush();
	}
}

void QGS_Router::route(QGS_ModuleMsgBase::ptr msg)
{

	if(msg->getDestination() == mName)
	{
		switch (msg->getType())
		{
		case messageTypes_t::Msg_Stop_e:
			mStop = true;
			break;
		default:
			break;
		}
	}
	else
	{
		mLog.setMsgLogLevel(log_level::debug);
		mLog << "sending to " << msgAddrStr.at(msg->getDestination());
		mLog.flush();

		sendMsg(std::move(msg));
	}

}

} /* namespace QuadGS */

