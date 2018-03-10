/*
 * QuadDebugMsg.cpp
 *
 *  Created on: Jan 7, 2017
 *      Author: martin
 */

#include "QGS_DebugMsg.h"

namespace QuadGS {



QGS_DebugMsg::QGS_DebugMsg(const QGS_IoHeader& header)
: QGS_IoHeader(header)
{

}

QGS_DebugMsg::QGS_DebugMsg(uint8_t Control, const std::string payload)
: QGS_IoHeader(QGS_IoHeader::Debug, Control, 0, payload.size())
, mPayload(payload)
{
}


QGS_DebugMsg::QGS_DebugMsg(uint8_t control, const uint8_t* data, uint16_t length)
: QGS_IoHeader(QGS_IoHeader::Debug, control, 0, length)
, mPayload()
{
	BinaryIStream is(data, length);
	is >> *this;
}

std::string QGS_DebugMsg::GetPayload() const
{
	return mPayload;
}

void QGS_DebugMsg::Setpayload(std::string payload)
{
	mPayload = payload;
	mPayloadSize = payload.size();
}

std::string QGS_DebugMsg::toString() const
{
	std::string result;
	result += 	QGS_IoHeader::toString();
	result += "[" +  mPayload + "]";
	return result;
}

BinaryOStream& QGS_DebugMsg::stream(BinaryOStream& os) const
{
	QGS_IoHeader::stream(os);
	os << SetBytes(mPayloadSize) << mPayload.c_str();
	return os;
}

BinaryIStream& QGS_DebugMsg::stream(BinaryIStream& is)
{
	std::vector<char> v;
	v.reserve(mPayloadSize);
	is >> SetBytes(mPayloadSize) >> v.data();
	mPayload = std::string(v.data(), mPayloadSize);

	return is;
}

QGS_DebugMsg::~QGS_DebugMsg()
{
}

} /* namespace QuadGS */
