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

QGS_ModuleMsg::ptr QGS_ModuleMsg::Create(messageTypes_t messageType)
{
	ptr p(new QGS_ModuleMsg(messageType) );
	return p;
}

QGS_ModuleMsg::ptr QGS_ModuleMsg::Create(QGS_ModuleMsg& copy)
{
	ptr p(new QGS_ModuleMsg(copy) );
	return p;
}

QGS_ModuleMsg::QGS_ModuleMsg(messageTypes type)
:mType(type), mOriginatingPort(-1), mDestinationPort(-1)
{

}

QGS_ModuleMsg::QGS_ModuleMsg(const QGS_ModuleMsg& msg)
:mType(0), mOriginatingPort(-1), mDestinationPort(-1)
{
	mType = static_cast<uint8_t>(msg.getType());
	mOriginatingPort = msg.getOriginatingPort();
	mDestinationPort = msg.getDestinationPort();
}


QGS_ModuleMsg::~QGS_ModuleMsg()
{

}



void QGS_ModuleMsg::setOriginator(std::string originator)
{
	mOriginator = originator;
}

void QGS_ModuleMsg::setDestinationPort(int port)
{
	mDestinationPort = port;
}

int QGS_ModuleMsg::getDestinationPort() const
{
	return mDestinationPort;
}

void QGS_ModuleMsg::setOriginatingPort(int port)
{
	mOriginatingPort = port;
}

int QGS_ModuleMsg::getOriginatingPort() const
{
	return mOriginatingPort;
}


messageTypes_t QGS_ModuleMsg::getType() const
{
	return static_cast<messageTypes_t>(mType);
}

std::string QGS_ModuleMsg::toString() const
{
	std::stringstream ss;
	ss << "ModuleMsg type: " << mType << std::endl;
	return ss.str();
}

BinaryIStream& QGS_ModuleMsg::stream(BinaryIStream& is)
{
	is >>  SetBits(8)  >> mType;
	is >>  SetBits(16) >> mOriginatingPort;
	is >>  SetBits(16) >> mDestinationPort;
	is >> mOriginator;

	return is;
}

BinaryOStream& QGS_ModuleMsg::stream(BinaryOStream& os) const
{
	os <<  SetBits(8)  << mType;
	os <<  SetBits(16) << mOriginatingPort;
	os <<  SetBits(16) << mDestinationPort;
	os << mOriginator;

	return os;
}



QGS_ModuleSubMsg::QGS_ModuleSubMsg(messageTypes_t type,messageTypes_t subscription)
:QGS_ModuleMsg(type), mSubscription(subscription)
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
	QGS_ModuleMsg::stream(is);
	is >> mSubscription;
	return is;
}

BinaryOStream& QGS_ModuleSubMsg::stream(BinaryOStream& os) const
{
	QGS_ModuleMsg::stream(os);
	os << mSubscription;
	return os;
}

} /* namespace QuadGS */
