/*
 * QuadLogMsg.cpp
 *
 *  Created on: Nov 17, 2017
 *      Author: martin
 */

#include "QuadLogMsg.h"
namespace QuadGS {

QuadLogMsg::ptr QuadLogMsg::Create(const uint8_t* data, uint16_t length)
{
    ptr p(new QuadLogMsg(data, length) );
    return p;
}

QuadLogMsg::ptr QuadLogMsg::Create(std::string payload)
{
    ptr p(new QuadLogMsg(payload) );
    return p;
}

QuadLogMsg::ptr QuadLogMsg::Create()
{
    ptr p(new QuadLogMsg() );
    return p;
}

std::string QuadLogMsg::GetPayload() const
{
    return mPayload;
}

void QuadLogMsg::Setpayload(std::string payload)
{
    mPayload = payload;
}

std::string QuadLogMsg::toString() const
{
    std::string result;
    result += "[" +  mPayload + "]";
    return result;
}

BinaryOStream& QuadLogMsg::stream(BinaryOStream& os) const
{
    os << mPayload;
    return os;
}

BinaryIStream& QuadLogMsg::stream(BinaryIStream& os)
{
    os >> mPayload;
    return os;
}

QuadLogMsg::~QuadLogMsg()
{
    // TODO Auto-generated destructor stub
}

BinaryOStream& operator <<(BinaryOStream& os, const QuadLogMsg& pl)
{
  pl.stream(os);
  return os;
}

BinaryIStream& operator >>(BinaryIStream& is, QuadLogMsg& pl)
{
  pl.stream(is);
  return is;
}

QuadLogMsg::QuadLogMsg()
    :QuadGSMsg()
{
    // TODO Auto-generated constructor stub

}

QuadLogMsg::QuadLogMsg(const std::string payload)
    :QuadGSMsg()
    ,mPayload(payload)
{

}


QuadLogMsg::QuadLogMsg(const uint8_t* data, uint16_t length)
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
