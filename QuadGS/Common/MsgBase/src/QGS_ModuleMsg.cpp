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

QGS_ModuleMsgBase::QGS_ModuleMsgBase()
:mType(0),
 mSource(""),
 mDestination(""),
 mMsgNr(0)
{

}

QGS_ModuleMsgBase::QGS_ModuleMsgBase(messageTypes type, const std::string& desination)
:mType(type), mSource(), mDestination(desination), mMsgNr(0)
{

}

QGS_ModuleMsgBase::QGS_ModuleMsgBase(const QGS_ModuleMsgBase& msg)
:mType(static_cast<uint8_t>(msg.getType()))
, mSource(msg.getSource())
, mDestination(msg.getDestination())
, mMsgNr(0)
{

}


QGS_ModuleMsgBase::~QGS_ModuleMsgBase()
{

}



void QGS_ModuleMsgBase::setSource(std::string originator)
{
	mSource = originator;
}

std::string QGS_ModuleMsgBase::getSource() const
{
	return mSource;;
}

void QGS_ModuleMsgBase::setDestination(std::string port)
{
	mDestination = port;
}

std::string QGS_ModuleMsgBase::getDestination() const
{
	return mDestination;
}

messageTypes_t QGS_ModuleMsgBase::getType() const
{
	return static_cast<messageTypes_t>(mType);
}

void QGS_ModuleMsgBase::setMsgNr(uint8_t nr)
{
	mMsgNr = nr;
}

uint8_t QGS_ModuleMsgBase::getMsgNr() const
{
	return mMsgNr;
}

void QGS_ModuleMsgBase::setSkipStreamHeader()
{
	mSkipStreamHeader = true;
}


std::string QGS_ModuleMsgBase::toString() const
{
	std::stringstream ss;
	ss << "ModuleMsg type: " << mType << std::endl;
	ss << "Source: " << mSource << std::endl;
	ss << "Destination: " << mDestination << std::endl;

	return ss.str();
}

BinaryIStream& QGS_ModuleMsgBase::stream(BinaryIStream& is)
{
	if(!mSkipStreamHeader)
	{
		mDestination.clear();
		mSource.clear();
		is >> SetBits(32)  >> mType;
		is >> mDestination;
		is >> mSource;
		mSkipStreamHeader = false;
	}
	return is;
}

BinaryOStream& QGS_ModuleMsgBase::stream(BinaryOStream& os) const
{
	os << SetBits(32)  << mType;
	os << mDestination;
	os << mSource;

	return os;
}

} /* namespace QuadGS */
