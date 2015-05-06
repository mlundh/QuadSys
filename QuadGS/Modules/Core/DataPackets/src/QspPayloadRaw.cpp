/*
 * QspPayloadBase.cpp
 *
 *  Created on: Feb 15, 2015
 *      Author: martin
 */

#include <string.h>

#include "../inc/QspPayloadRaw.h"
namespace QuadGS {


QspPayloadRaw::QspPayloadRaw(uint8_t* Payload, uint8_t PayloadLength):
    mPayloadLength(PayloadLength)
    , mArrayLength(PayloadLength)
{
    mPayload = new uint8_t[PayloadLength];
    memcpy(mPayload, Payload, PayloadLength * sizeof(uint8_t));
}

QspPayloadRaw::QspPayloadRaw(std::size_t PayloadLength):
    mPayloadLength(PayloadLength)
    , mArrayLength(PayloadLength)
{
    mPayload = new uint8_t[PayloadLength];
}

// Create a new QspPayloadBase that is a copy of this instance.
QspPayloadRaw::Ptr QspPayloadRaw::Copy()
{
    return Create(mPayload, mPayloadLength);
}

QspPayloadRaw::~QspPayloadRaw()
{
    delete[] mPayload;
}

uint8_t& QspPayloadRaw::operator[](std::size_t idx)
{
    if(idx < mArrayLength)
    {
        return mPayload[idx];
    }
    else
    {
        //TODO how to handle error?
        return mPayload[mArrayLength - 1];
    }
}
uint8_t QspPayloadRaw::operator[](std::size_t idx) const
{
    if(idx < mArrayLength)
    {
        return mPayload[idx];
    }
    else
    {
        //TODO how to handle error?
        return mPayload[mArrayLength - 1];
    }

}


QspPayloadRaw::Ptr QspPayloadRaw::Create(uint8_t* Payload, uint8_t PayloadLength)
{
    Ptr p(new QspPayloadRaw(Payload, PayloadLength) );
    return p;
}

QspPayloadRaw::Ptr QspPayloadRaw::Create(std::size_t PayloadLength)
{
    Ptr p(new QspPayloadRaw(PayloadLength) );
    return p;
}

uint8_t* QspPayloadRaw::getPayload()
{
  return mPayload;
}

std::size_t QspPayloadRaw::getPayloadLength()
{
    return mPayloadLength;
}
bool QspPayloadRaw::setPayloadLength(std::size_t size)
{
    if(size >= mArrayLength)
    {
        return false; //can not have a larger payload than the allocated array.
    }
    mPayloadLength = size;
    return true;
}


} /* namespace QuadGS */
