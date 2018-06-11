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


/*
QGSParamMsg::QGSParamMsg(const uint8_t* data, uint16_t length, uint8_t sequenceNr, uint8_t lastInSequence)
	: QGS_MsgHeader()
	, mSequenceNumber(sequenceNr)
	, mLastInSequence(lastInSequence)
	, mPayload((const char*)data, (size_t)length)
{

}*/


QGSParamMsg::QGSParamMsg(const QGS_IoHeader& header, const uint8_t* data, uint16_t length)
	: QGS_IoHeader(header)
	, mSequenceNumber(0)
	, mLastInSequence(0)
	, mPayload()
{
	BinaryIStream is1(data, length);
	mPayloadSize = length;
	is1 >> *this;
}

QGSParamMsg::QGSParamMsg(const QGS_IoHeader& header)
	: QGS_IoHeader(header)
	, mSequenceNumber(0)
	, mLastInSequence(0)
	, mPayload()
{
}

QGSParamMsg::QGSParamMsg(const QGSParamMsg& msg)
	: QGS_IoHeader(msg)
	, mSequenceNumber(msg.mSequenceNumber)
	, mLastInSequence(msg.mLastInSequence)
	, mPayload(msg.GetPayload())
{
}

QGSParamMsg::QGSParamMsg(uint8_t Control, const std::string payload, uint8_t sequenceNr, uint8_t lastInSequence)
	: QGS_IoHeader( Parameters, Control, 0, payload.length() + 1)
	, mSequenceNumber(sequenceNr)
	, mLastInSequence(lastInSequence)
	, mPayload(payload)
{

}

QGSParamMsg::~QGSParamMsg()
{
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
	mPayloadSize = payload.length() + 1;
}

std::string QGSParamMsg::toString() const
{
	std::string result;
	result += 	QGS_IoHeader::toString();
	result += "[" + std::to_string(GetLastInSeq()) + "]";
	result += "[" + std::to_string(GetSequenceNumber()) + "]";
	result += "[" +  mPayload + "]";
	return result;
}

BinaryOStream& QGSParamMsg::stream(BinaryOStream& os) const
{
	QGS_IoHeader::stream(os);
	os <<  SetBits(1) << mLastInSequence;
	os <<  SetBits(7) << mSequenceNumber;
	os <<  SetBytes(mPayloadSize -1) << mPayload.c_str();
	return os;
}

BinaryIStream& QGSParamMsg::stream(BinaryIStream& is)
{
	is >> SetBits(1) >> mLastInSequence;
	is >> SetBits(7) >> mSequenceNumber;
	std::vector<char> v;
	v.reserve(mPayloadSize - 1);
	is >> SetBytes(mPayloadSize - 1) >> v.data();
	mPayload = std::string(v.data(), mPayloadSize - 1);
	return is;
}


} /* namespace QuadGS */
