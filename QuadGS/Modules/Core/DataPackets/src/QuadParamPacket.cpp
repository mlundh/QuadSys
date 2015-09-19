/*
 * QuadParamPacket.cpp
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

#include "QuadParamPacket.h"

namespace QuadGS {


QuadParamPacket::~QuadParamPacket()
{
}

QuadParamPacket::Ptr QuadParamPacket::Create(const uint8_t* data, uint16_t length)
{
    Ptr p(new QuadParamPacket(data, length) );
    return p;
}
QuadParamPacket::Ptr QuadParamPacket::Create(const QspPayloadRaw::Ptr packet)
{
    Ptr p(new QuadParamPacket(packet) );
    return p;
}

uint8_t QuadParamPacket::GetLastInSeq()
{
    uint8_t tmp = DeserializeInt8(4);
    return ((tmp >> 7) & 1 ); // Highest bit indicates last in sequence.
}
void QuadParamPacket::SetLastInSeq(uint8_t isLast)
{
    isLast &= 1;
    uint8_t currentVal = DeserializeInt8(4);
    currentVal = (currentVal & ~(1 << 7)) | (isLast << 7); // Clear bit, then set to isLast.
    SerializeInt8(currentVal, 4);
}

uint8_t QuadParamPacket::GetSequenceNumber()
{
    uint8_t tmp = DeserializeInt8(4);
    tmp &= ~(1 << 7);
    return tmp;
}
void QuadParamPacket::SetSequenceNumber(uint8_t sequenceNumber)
{
    uint8_t tmp = DeserializeInt8(4);
    tmp &= ~(0x7f);                 // Clear sequence nr from stored value.
    sequenceNumber &= ~(1 << 7);    // Clear top bit from sequenceNumber (should have been 0 already).
    sequenceNumber |= tmp;          // Combine the two.
    SerializeInt8(sequenceNumber, 4);
}

uint16_t QuadParamPacket::GetPayloadLength()
{
    uint16_t result = QuadSerialPacket::GetPayloadLength();
    if(result < 1)
    {
        throw std::runtime_error("Not a param packet.");
    }
    return result - 1;
}


QspPayloadRaw::Ptr QuadParamPacket::GetPayload()
{
    // Copy payload to new QspPayloadRaw object.
    return QspPayloadRaw::Create(mPayload->getPayload() + mHeaderSize + 1, GetPayloadLength());
}

std::string QuadParamPacket::PayloadToString()
{
    std::string result;
    result += "[" + std::to_string(GetLastInSeq()) + "]";
    result += "[" + std::to_string(GetSequenceNumber()) + "]";
    std::string str = mPayload->toString(mHeaderSize + 1);
    result += "[" +  str + "]";
    return result;
}

QuadParamPacket::QuadParamPacket(const uint8_t* data, uint16_t length, uint8_t sequenceNr):
        QuadSerialPacket(data, length, 1)
{
    SetSequenceNumber(sequenceNr);
}


QuadParamPacket::QuadParamPacket(const uint8_t* data, uint16_t length):
        QuadSerialPacket(data, length, 1)
{

}

QuadParamPacket::QuadParamPacket(const QspPayloadRaw::Ptr packet):
        QuadSerialPacket(packet)
{

}


} /* namespace QuadGS */
