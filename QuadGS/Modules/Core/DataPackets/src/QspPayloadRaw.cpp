/*
 * QspPayloadBase.cpp
 *
 *  Created on: Feb 15, 2015
 *      Author: martin
 */

#include <string.h>

#include "QspPayloadRaw.h"
namespace QuadGS {


QspPayloadRaw::QspPayloadRaw(const uint8_t* Payload, uint8_t PayloadLength):
    mPayloadLength(PayloadLength)
    , mArrayLength(255)
{
    mPayload = new uint8_t[mArrayLength];
    memcpy(mPayload, Payload, PayloadLength * sizeof(uint8_t));
}

QspPayloadRaw::QspPayloadRaw(std::size_t PayloadLength):
    mPayloadLength(PayloadLength)
    , mArrayLength(255)
{
    mPayload = new uint8_t[mArrayLength];
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
        throw std::out_of_range("Payload index " + std::to_string(idx) + "does not exist, max: " + std::to_string(mArrayLength) + ".");
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
      throw std::out_of_range("Payload index " + std::to_string(idx) + "does not exist, max: " + std::to_string(mArrayLength) + ".");
    }

}

std::ostream& operator <<(std::ostream& stream, const QspPayloadRaw& pl) 
{
  stream << "Payload length: " << std::dec << pl.getPayloadLength() << std::endl << "Payload data: ";
  for(size_t i = 0; i < pl.getPayloadLength(); i++)
  {
    stream << std::hex << pl[i];
  }
  return stream;
}



QspPayloadRaw::Ptr QspPayloadRaw::Create(const uint8_t* Payload, uint8_t PayloadLength)
{
    Ptr p(new QspPayloadRaw(Payload, PayloadLength) );
    return p;
}

QspPayloadRaw::Ptr QspPayloadRaw::Create(std::size_t PayloadLength)
{
    Ptr p(new QspPayloadRaw(PayloadLength) );
    return p;
}

uint8_t* QspPayloadRaw::getPayload() const
{
  return mPayload;
}



std::size_t QspPayloadRaw::getPayloadLength() const
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
