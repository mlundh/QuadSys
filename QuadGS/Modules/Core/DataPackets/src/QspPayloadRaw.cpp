/*
 * QspPayloadBase.cpp
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

#include <string.h>
#include <sstream>
#include "QspPayloadRaw.h"
namespace QuadGS {

const size_t QspPayloadRaw::mArrayLength(512);

QspPayloadRaw::QspPayloadRaw(const uint8_t* Payload, uint16_t PayloadLength, uint16_t offset):
    mPayloadLength(static_cast<uint16_t>(PayloadLength + offset))
{
    mPayload = new uint8_t[mArrayLength];
    if(Payload)
    {
        memcpy((mPayload + offset), Payload, PayloadLength * sizeof(uint8_t));
    }
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
  stream << std::hex << pl.toString();
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

std::string QspPayloadRaw::toString(int offset) const
{
    if(getPayloadLength() <= offset)
    {
        return "";
    }
    std::string str(mPayload + offset, mPayload + getPayloadLength());
    return str;
}


} /* namespace QuadGS */
