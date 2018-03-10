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
namespace QuadGS
{

QGS_Router::QGS_Router(std::string name):QGS_MessageHandlerBase(name), mNrModules(0), mName(name), mStop(false)
{
	mThread = std::thread(std::bind(&QGS_Router::runRouter, this));
}

QGS_Router::~QGS_Router()
{
	QGS_ModuleMsg::ptr ptr  = std::make_unique<QGS_ModuleMsg>(messageTypes_t::msgQuit);
	mFifo.push(std::move(ptr)); // send stop to own fifo.

	if(mThread.joinable())
	{
		mThread.join();
	}
}

void QGS_Router::bind(QGS_Module* module)
{
	module->setSendFunc(std::bind(&QGS_Router::incomingPort, this, std::placeholders::_1, mNrModules++)); //bind port number to the caller
	checkUniqueName(module->getName());
	mWriteFunctions.push_back(module->getReceivingFcn());
	mPortNameMaping.push_back(module->getName());
}

bool QGS_Router::done()
{
	return mFifo.empty();
}


void QGS_Router::incomingPort(QGS_ModuleMsg::ptr message, int port)
{
	message->setOriginatingPort(port);
	mFifo.push(std::move(message));
}


void QGS_Router::sendMsg(QGS_ModuleMsg::ptr message)
{
	messageTypes_t type = message->getType();
	if(mSubscriptions.find(type) != mSubscriptions.end())
	{
		// If we already know where to send to, then send.
		int destPort = message->getDestinationPort();
		if(destPort >= 0) // Did we get a destination?
		{
			internalSend(std::move(message), destPort);
		}
		else // Did not get a destination, send to all subscribers.
		{
			int originatingPort = message->getOriginatingPort();
			for(auto subsPort : mSubscriptions[type])
			{
				if(subsPort != originatingPort) // Do not return to sender
				{
					internalSend(std::move(message), subsPort);
				}
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


void QGS_Router::internalSend(QGS_ModuleMsg::ptr message, int port)
{
	//Make sure we have a write function.
	if((mWriteFunctions.size() >= static_cast<size_t>(port)) && (mWriteFunctions[port] != NULL))
	{
		// each module gets its own copy. This ensures thread safety. Naive but works...
		QGS_ModuleMsg::ptr ptr = QGS_ModuleMsg::Create(*message);
		mWriteFunctions[port](std::move(ptr));
	}
}


void QGS_Router::runRouter()
{
	while(!mStop)
	{

		QGS_ModuleMsg::ptr msg(mFifo.dequeue());
		route(std::move(msg));
	}
	mLogger.QuadLog(severity_level::info, "Stopping router.");

}


void QGS_Router::checkUniqueName(std::string &name)
{
	for (auto& item : mPortNameMaping)
	{
		if(item == name)
		{
			mLogger.QuadLog(severity_level::warning, "Name collision, two modules named: " + name + ". Advice to change name.");
		}
	}
}

void QGS_Router::route(QGS_ModuleMsg::ptr msg)
{
	switch (msg->getType())
	{
	// This check is only done since we only want subscription messages, but receives and forwards all other.
	// Normal modules will just call msg->dispatch(this) on the incoming message.
	case messageTypes_t::msgSubscription:
	{
		msg->dispatch(this);
	}
	break;

	case msgQuit:
	{
		mStop = true;
	}
	break;
	default:
		sendMsg(std::move(msg));
		break;
	}

}
void QGS_Router::process(QGS_ModuleSubMsg* msg)
{
	mSubscriptions[msg->getSubscription()].push_back(msg->getOriginatingPort());
}

} /* namespace QuadGS */

