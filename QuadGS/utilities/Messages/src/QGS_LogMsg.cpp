/*
 * QuadLogMsg.cpp
 *
 *  Created on: Nov 17, 2017
 *      Author: martin
 */

#include "QGS_LogMsg.h"
namespace QuadGS {

QGS_LogMsg::ptr QGS_LogMsg::Create(const uint8_t* data, uint16_t length)
{
    ptr p(new QGS_LogMsg(data, length) );
    return p;
}

QGS_LogMsg::ptr QGS_LogMsg::Create(std::string payload)
{
    ptr p(new QGS_LogMsg(payload) );
    return p;
}

QGS_LogMsg::ptr QGS_LogMsg::Create()
{
    ptr p(new QGS_LogMsg() );
    return p;
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
    os << mPayload;
    return os;
}

BinaryIStream& QGS_LogMsg::stream(BinaryIStream& os)
{
    os >> mPayload;
    return os;
}

QGS_LogMsg::~QGS_LogMsg()
{
    // TODO Auto-generated destructor stub
}


QGS_LogMsg::QGS_LogMsg()
    :QGS_Msg()
{
    // TODO Auto-generated constructor stub

}

QGS_LogMsg::QGS_LogMsg(const std::string payload)
    :QGS_Msg()
    ,mPayload(payload)
{

}


QGS_LogMsg::QGS_LogMsg(const uint8_t* data, uint16_t length)
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
