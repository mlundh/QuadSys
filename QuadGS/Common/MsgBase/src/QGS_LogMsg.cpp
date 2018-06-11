/*
 * QuadLogMsg.cpp
 *
 *  Created on: Nov 17, 2017
 *      Author: martin
 */

#include "QGS_LogMsg.h"
namespace QuadGS {

QGS_LogMsg::QGS_LogMsg(const QGS_IoHeader& header)
:QGS_IoHeader(header)
{

}

QGS_LogMsg::QGS_LogMsg(uint8_t Control, const std::string payload)
: QGS_IoHeader(QGS_IoHeader::Log, Control, 0, payload.size())
,mPayload(payload)
{

}


QGS_LogMsg::QGS_LogMsg(uint8_t Control, const uint8_t* data, uint16_t length)
: QGS_IoHeader(QGS_IoHeader::Log, Control, 0, length)
, mPayload()
{
	BinaryIStream is(data, length);
	is >> *this;
}

std::string QGS_LogMsg::GetPayload() const
{
	return mPayload;
}

void QGS_LogMsg::Setpayload(std::string payload)
{
	mPayload = payload;
}

std::string QGS_LogMsg::toString() const
{
	std::string result;
	result += "[" +  mPayload + "]";
	return result;
}

BinaryOStream& QGS_LogMsg::stream(BinaryOStream& os) const
{
	os << SetBytes(mPayloadSize) << mPayload;
	return os;
}

BinaryIStream& QGS_LogMsg::stream(BinaryIStream& is)
{
	std::vector<char> v;
	v.reserve(mPayloadSize);
	is >> SetBytes(mPayloadSize) >> v.data();
	mPayload = std::string(v.data(), mPayloadSize);

	return is;
}

QGS_LogMsg::~QGS_LogMsg()
{
	// TODO Auto-generated destructor stub
}


} /* namespace QuadGS */
