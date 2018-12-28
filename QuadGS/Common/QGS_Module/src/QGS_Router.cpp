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

QGS_Router::QGS_Router(msgAddr_t name): mNrModules(0), mName(name), mStop(false), mLogger(msgAddrStr[name])
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
	msgAddr_t name = module->getName(); // TODO add support for multiple addresses/names per module (normally only one, but usage for IO)
	checkUniqueName(name);
	mWriteFunctions[name] = module->getReceivingFcn();
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
	messageTypes_t type = message->getType();
		// If we already know where to send to, then send.
		msgAddr_t dest = message->getDestination();
		if(dest != msgAddr_t::Unassigned && dest != msgAddr_t::Broadcast) // Did we get a destination?
		{
			internalSend(std::move(message), dest, false);
		}
		else if(dest == msgAddr_t::Broadcast) // Did not get a destination, send to all subscribers.
		{
			msgAddr_t source = message->getSource();
			for(auto module : mWriteFunctions)
			{
				if(module.first != source) // Do not return to sender
				{
					internalSend(std::move(message), module.first, true);
				}
			}
		}

	else
	{
		std::stringstream ss;
		ss << "No subscriber to message type: " << type;
		mLogger.QuadLog(severity_level::warning, ss.str() );
	}
}


void QGS_Router::internalSend(QGS_ModuleMsgBase::ptr message, msgAddr_t port, bool broadcast)
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
			mLogger.QuadLog(severity_level::error, "Module: " + msgAddrStr[port] + " encountered an error: " + e.what());
		}
	}
	else
	{
		mLogger.QuadLog(severity_level::error, "No port: " + msgAddrStr[port] + ", error in topology? ");
	}
}


void QGS_Router::runRouter()
{
	while(!mStop)
	{

		QGS_ModuleMsgBase::ptr msg(mFifo.dequeue());
		route(std::move(msg));
	}
	mLogger.QuadLog(severity_level::info, "Stopping router.");

}


void QGS_Router::checkUniqueName(msgAddr_t name)
{
	std::map<msgAddr_t,WriteFcn>::iterator it = mWriteFunctions.find(name);
	if(it != mWriteFunctions.end())
	{
		mLogger.QuadLog(severity_level::warning, "Name collision, two modules named: " + msgAddrStr[name] + ". Advice to change name.");
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
			case messageTypes_t::Msg_RegisterName_e:
			{

				Msg_RegisterName* nameMsg = dynamic_cast<Msg_RegisterName*>(msg.get());
				if(!nameMsg)
				{
					throw std::runtime_error("No conversion possible, ERROR! ");
				}
				msgAddr_t name = static_cast<msgAddr_t>(nameMsg->getName());
				checkUniqueName(name);

				// Now get the receiving function of the registering module, it exists in the writeFunction map.
				WriteFcn fcn = mWriteFunctions[msg->getSource()];
				mWriteFunctions[name] = fcn;
			}
				break;
			default:
				break;
		}
	}
	else
	{
		mLogger.QuadLog(severity_level::debug, "sending to " + msgAddrStr[msg->getDestination()]);
		sendMsg(std::move(msg));
	}

}

} /* namespace QuadGS */

