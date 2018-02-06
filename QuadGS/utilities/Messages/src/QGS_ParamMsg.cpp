/*
 * QuadParamPacket.cpp
 *
 * Copyright (C) 2015 Martin Lundh
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

#include "QGS_ParamMsg.h"

namespace QuadGS {



QGSParamMsg::~QGSParamMsg()
{
}

QGSParamMsg::ptr QGSParamMsg::Create(const uint8_t* data, uint16_t length)
{
	ptr p(new QGSParamMsg(data, length) );
	return p;
}

QGSParamMsg::ptr QGSParamMsg::Create()
{
	ptr p(new QGSParamMsg() );
	return p;
}

QGSParamMsg::ptr QGSParamMsg::Create(const uint8_t* data, uint16_t length, uint8_t sequenceNr, uint8_t lastInSequence)
{
	ptr p(new QGSParamMsg(data, length, sequenceNr, lastInSequence) );
	return p;
}


QGSParamMsg::ptr QGSParamMsg::Create(const std::string data, uint8_t sequenceNr, uint8_t lastInSequence)
{
	ptr p(new QGSParamMsg(data, sequenceNr, lastInSequence) );
	return p;
}
uint8_t QGSParamMsg::GetLastInSeq() const
{
	return mLastInSequence;
}
void QGSParamMsg::SetLastInSeq(uint8_t isLast)
{
	isLast &= 1;
	mLastInSequence = isLast;
}

uint8_t QGSParamMsg::GetSequenceNumber() const
{
	return mSequenceNumber;
}
void QGSParamMsg::SetSequenceNumber(uint8_t sequenceNumber)
{
	mSequenceNumber = sequenceNumber;
}

std::string QGSParamMsg::GetPayload() const
{
	return mPayload;
}
void QGSParamMsg::Setpayload(std::string payload)
{
	mPayload = payload;
}

std::string QGSParamMsg::toString() const
{
	std::string result;
	result += "[" + std::to_string(GetLastInSeq()) + "]";
	result += "[" + std::to_string(GetSequenceNumber()) + "]";
	result += "[" +  mPayload + "]";
	return result;
}

BinaryOStream& QGSParamMsg::stream(BinaryOStream& os) const
{
	os <<  SetBits(1) << mLastInSequence;
	os <<  SetBits(7) << mSequenceNumber;
	os << mPayload;
	return os;
}

BinaryIStream& QGSParamMsg::stream(BinaryIStream& is)
{
	is >> SetBits(1) >> mLastInSequence;
	is >> SetBits(7) >> mSequenceNumber;
	is >> mPayload;
	return is;
}

QGSParamMsg::QGSParamMsg(const uint8_t* data, uint16_t length, uint8_t sequenceNr, uint8_t lastInSequence)
	: QGS_Msg()
	, mSequenceNumber(sequenceNr)
	, mLastInSequence(lastInSequence)
	, mPayload((const char*)data, (size_t)length)
{

}


QGSParamMsg::QGSParamMsg(const uint8_t* data, uint16_t length)
	: QGS_Msg()
	, mSequenceNumber(0)
	, mLastInSequence(0)
	, mPayload()
{
	BinaryOStream os;
	for(int i = 0; i < length; i++)
	{
		os << data[i];
	}
	BinaryIStream is(os.get_internal_vec());
	is >> *this;
}

QGSParamMsg::QGSParamMsg()
	: QGS_Msg()
	, mSequenceNumber(0)
	, mLastInSequence(0)
	, mPayload()
{
}


QGSParamMsg::QGSParamMsg(const std::string payload, uint8_t sequenceNr, uint8_t lastInSequence)
	: QGS_Msg()
	, mSequenceNumber(sequenceNr)
	, mLastInSequence(lastInSequence)
	, mPayload(payload)
{

}

BinaryOStream& operator <<(BinaryOStream& os, const QGSParamMsg& pl)
{
  pl.stream(os);
  return os;
}

BinaryIStream& operator >>(BinaryIStream& is, QGSParamMsg& pl)
{
  pl.stream(is);
  return is;
}

} /* namespace QuadGS */
