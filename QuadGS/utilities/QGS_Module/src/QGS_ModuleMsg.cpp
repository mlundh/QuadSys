/*
 * QGS_ModuleMsg.cpp
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

#include "QGS_ModuleMsg.h"
#include <sstream>
#include <iostream>

namespace QuadGS {

QGS_ModuleMsgBase::QGS_ModuleMsgBase(messageTypes type)
:mType(type), mOriginatingPort(-1), mDestinationPort(-1), mOriginator()
{

}

QGS_ModuleMsgBase::QGS_ModuleMsgBase(const QGS_ModuleMsgBase& msg)
:mType(static_cast<uint8_t>(msg.getType()))
, mOriginatingPort( msg.getOriginatingPort())
, mDestinationPort(msg.getDestinationPort())
, mOriginator(msg.getOriginator())
{

}


QGS_ModuleMsgBase::~QGS_ModuleMsgBase()
{

}



void QGS_ModuleMsgBase::setOriginator(std::string originator)
{
	mOriginator = originator;
}

std::string QGS_ModuleMsgBase::getOriginator() const
{
	return mOriginator;;
}

void QGS_ModuleMsgBase::setDestinationPort(int port)
{
	mDestinationPort = port;
}

int QGS_ModuleMsgBase::getDestinationPort() const
{
	return mDestinationPort;
}

void QGS_ModuleMsgBase::setOriginatingPort(int port)
{
	mOriginatingPort = port;
}

int QGS_ModuleMsgBase::getOriginatingPort() const
{
	return mOriginatingPort;
}


messageTypes_t QGS_ModuleMsgBase::getType() const
{
	return static_cast<messageTypes_t>(mType);
}

std::string QGS_ModuleMsgBase::toString() const
{
	std::stringstream ss;
	ss << "ModuleMsg type: " << mType << std::endl;
	return ss.str();
}

BinaryIStream& QGS_ModuleMsgBase::stream(BinaryIStream& is)
{
	is >>  SetBits(8)  >> mType;
	is >>  SetBits(16) >> mOriginatingPort;
	is >>  SetBits(16) >> mDestinationPort;
	is >> mOriginator;

	return is;
}

BinaryOStream& QGS_ModuleMsgBase::stream(BinaryOStream& os) const
{
	os <<  SetBits(8)  << mType;
	os <<  SetBits(16) << mOriginatingPort;
	os <<  SetBits(16) << mDestinationPort;
	os << mOriginator;

	return os;
}



QGS_ModuleSubMsg::QGS_ModuleSubMsg(messageTypes_t subscription)
:QGS_ModuleMsg(messageTypes_t::msgSubscription), mSubscription(subscription)
{

}

QGS_ModuleSubMsg::QGS_ModuleSubMsg(const QGS_ModuleSubMsg& msg)
:QGS_ModuleMsg(msg), mSubscription(msg.mSubscription)
{

}
QGS_ModuleSubMsg::~QGS_ModuleSubMsg()
{

}

messageTypes_t QGS_ModuleSubMsg::getSubscription()
{
	return (messageTypes_t)mSubscription;
}
void QGS_ModuleSubMsg::setSubscription(messageTypes_t type)
{
	mSubscription = type;
}


BinaryIStream& QGS_ModuleSubMsg::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mSubscription;
	return is;
}

BinaryOStream& QGS_ModuleSubMsg::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mSubscription;
	return os;
}





} /* namespace QuadGS */
