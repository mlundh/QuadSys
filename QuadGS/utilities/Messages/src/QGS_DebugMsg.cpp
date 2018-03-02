/*
 * QuadDebugMsg.cpp
 *
 *  Created on: Jan 7, 2017
 *      Author: martin
 */

#include "QGS_DebugMsg.h"

namespace QuadGS {

QGS_DebugMsg::ptr QGS_DebugMsg::Create(const uint8_t* data, uint16_t length)
{
	ptr p(new QGS_DebugMsg(data, length) );
	return p;
}

QGS_DebugMsg::ptr QGS_DebugMsg::Create(std::string payload)
{
	ptr p(new QGS_DebugMsg(payload) );
	return p;
}

QGS_DebugMsg::ptr QGS_DebugMsg::Create()
{
	ptr p(new QGS_DebugMsg() );
	return p;
}

std::string QGS_DebugMsg::GetPayload() const
{
	return mPayload;
}

void QGS_DebugMsg::Setpayload(std::string payload)
{
	mPayload = payload;
}

std::string QGS_DebugMsg::toString() const
{
	std::string result;
	result += "[" +  mPayload + "]";
	return result;
}

BinaryOStream& QGS_DebugMsg::stream(BinaryOStream& os) const
{
	os << mPayload;
	return os;
}

BinaryIStream& QGS_DebugMsg::stream(BinaryIStream& os)
{
	os >> mPayload;
	return os;
}

QGS_DebugMsg::~QGS_DebugMsg()
{
	// TODO Auto-generated destructor stub
}
/*
BinaryOStream& operator <<(BinaryOStream& os, const QGS_DebugMsg& pl)
{
  pl.stream(os);
  return os;
}

BinaryIStream& operator >>(BinaryIStream& is, QGS_DebugMsg& pl)
{
  pl.stream(is);
  return is;
}
*/
QGS_DebugMsg::QGS_DebugMsg()
	:QGS_Msg()
{
	// TODO Auto-generated constructor stub

}

QGS_DebugMsg::QGS_DebugMsg(const std::string payload)
	:QGS_Msg()
	,mPayload(payload)
{

}


QGS_DebugMsg::QGS_DebugMsg(const uint8_t* data, uint16_t length)
	: QGS_Msg()
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
} /* namespace QuadGS */
