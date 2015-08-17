/*
 * QspPayloadBase.cpp
 *
 *  Created on: Feb 15, 2015
 *      Author: martin
 */

#include <string.h>
#include <sstream>
#include "QspPayloadRaw.h"
namespace QuadGS {

const size_t QspPayloadRaw::mArrayLength(512);

QspPayloadRaw::QspPayloadRaw(const uint8_t* Payload, uint16_t PayloadLength, uint16_t offset):
    mPayloadLength(static_cast<uint16_t>(PayloadLength + offset))
{
    mPayload = new uint8_t[mArrayLength];
    memcpy((mPayload + offset), Payload, PayloadLength * sizeof(uint8_t));
}

QspPayloadRaw::QspPayloadRaw(uint16_t PayloadLength):
    mPayloadLength(PayloadLength)
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

QspPayloadRaw::Ptr QspPayloadRaw::Create(const uint8_t* Payload, uint16_t PayloadLength, uint16_t offset)
{
  Ptr p(new QspPayloadRaw(Payload, PayloadLength, offset) );
  return p;
}


QspPayloadRaw::Ptr QspPayloadRaw::Create(const uint8_t* Payload, uint16_t PayloadLength)
{
    Ptr p(new QspPayloadRaw(Payload, PayloadLength, 0) );
    return p;
}

QspPayloadRaw::Ptr QspPayloadRaw::Create(uint16_t PayloadLength)
{
    Ptr p(new QspPayloadRaw(PayloadLength) );
    return p;
}

uint8_t* QspPayloadRaw::getPayload() const
{
  return mPayload;
}



uint16_t QspPayloadRaw::getPayloadLength() const
{
    return mPayloadLength;
}
bool QspPayloadRaw::setPayloadLength(uint16_t size)
{
    if(size >= mArrayLength)
    {
        return false; //can not have a larger payload than the allocated array.
    }
    mPayloadLength = size;
    return true;
}

std::string QspPayloadRaw::toString()
{
    std::string str(mPayload, mPayload + getPayloadLength());
    return str;
}


} /* namespace QuadGS */
