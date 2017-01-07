/*
 * QuadDebugMsg.cpp
 *
 *  Created on: Jan 7, 2017
 *      Author: martin
 */

#include "../QuadDebugMsg.h"

namespace QuadGS {

QuadDebugMsg::ptr QuadDebugMsg::Create(const uint8_t* data, uint16_t length)
{
	ptr p(new QuadDebugMsg(data, length) );
	return p;
}

QuadDebugMsg::ptr QuadDebugMsg::Create(std::string payload)
{
	ptr p(new QuadDebugMsg(payload) );
	return p;
}

QuadDebugMsg::ptr QuadDebugMsg::Create()
{
	ptr p(new QuadDebugMsg() );
	return p;
}

std::string QuadDebugMsg::GetPayload() const
{
	return mPayload;
}

void QuadDebugMsg::Setpayload(std::string payload)
{
	mPayload = payload;
}

std::string QuadDebugMsg::toString() const
{
	std::string result;
	result += "[" +  mPayload + "]";
	return result;
}

BinaryOStream& QuadDebugMsg::stream(BinaryOStream& os) const
{
	os << mPayload;
	return os;
}

BinaryIStream& QuadDebugMsg::stream(BinaryIStream& os)
{
	os >> mPayload;
	return os;
}

QuadDebugMsg::~QuadDebugMsg()
{
	// TODO Auto-generated destructor stub
}

BinaryOStream& operator <<(BinaryOStream& os, const QuadDebugMsg& pl)
{
  pl.stream(os);
  return os;
}

BinaryIStream& operator >>(BinaryIStream& is, QuadDebugMsg& pl)
{
  pl.stream(is);
  return is;
}

QuadDebugMsg::QuadDebugMsg()
	:QuadGSMsg()
{
	// TODO Auto-generated constructor stub

}

QuadDebugMsg::QuadDebugMsg(const std::string payload)
	:QuadGSMsg()
	,mPayload(payload)
{

}


QuadDebugMsg::QuadDebugMsg(const uint8_t* data, uint16_t length)
	: QuadGSMsg()
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
