/*
 * QuadSerialPacket.cpp
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
#include "QuadSerialPacket.h"
#include <stdlib.h>



namespace QuadGS {


std::vector<std::string> QuadSerialPacket::mAddressStrings =
   {
           "Parameters",
           "Log",
           "FunctionCall",
           "Status",
           "Debug"
   };
std::vector<std::string> QuadSerialPacket::mParamControl =
    {
            "SetTree",
            "GetTree",
            "Value",
            "Save",
            "Load"
    };
std::vector<std::string> QuadSerialPacket::mStatusControl =
{
        "Cont",
        "Ack",
        "Nack",
        "Error",
        "NotAllowed",
        "UnexpectedSequence",
        "NotValidSlipPacket",
        "BufferOverrun"
};
std::vector<std::string> QuadSerialPacket::mDebugControl =
{
        "GetRuntimeStats",
        "SetRuntimeStats",
        "GetErrorMessages",
};


QuadSerialPacket::Ptr QuadSerialPacket::Create(const uint8_t* data, uint16_t length)
{
    Ptr p(new QuadSerialPacket(data, length) );
    return p;
}
QuadSerialPacket::Ptr QuadSerialPacket::Create(const QspPayloadRaw::Ptr packet)
{
    Ptr p(new QuadSerialPacket(packet) );
    return p;
}

QuadSerialPacket::~QuadSerialPacket()
{
}

uint8_t QuadSerialPacket::GetAddress()
{
    return DeserializeInt8(0);
}
void QuadSerialPacket::SetAddress(uint8_t address)
{
    SerializeInt8(address, 0);
}

uint8_t QuadSerialPacket::GetControl()
{
    uint8_t tmp = DeserializeInt8(1);
    tmp &= ~(1 << 7);
    return tmp;
}
void QuadSerialPacket::SetControl(uint8_t control)
{
    uint8_t tmp =  DeserializeInt8(1);
    tmp &= ~(0x7f);                 // Clear sequence nr from stored value.
    control &= ~(1 << 7);           // Clear top bit from sequenceNumber (should have been 0 already).
    control |= tmp;                 // Combine the two.
    SerializeInt8(control, 1);
}

uint8_t QuadSerialPacket::GetIsResend()
{
    uint8_t tmp = DeserializeInt8(1);
    return ((tmp >> 7) & 1 ); // Highest bit indicates IsResend.
}


void QuadSerialPacket::SetIsResend(uint8_t IsResend)
{
    IsResend &= 1;
    uint8_t currentVal = DeserializeInt8(1);
    currentVal = (currentVal & ~(1 << 7)) | (IsResend << 7); // Clear bit, then set to resend.
    SerializeInt8(currentVal, 1);
}


uint16_t QuadSerialPacket::GetPayloadLength()
{
    return DeserializeInt16(2);
}

QspPayloadRaw::Ptr QuadSerialPacket::GetPayload()
{
    // Copy payload to new QspPayloadRaw object.
    return QspPayloadRaw::Create(mPayload->getPayload() + mHeaderSize, GetPayloadLength());

}

QspPayloadRaw::Ptr QuadSerialPacket::GetRawData()
{
    return  mPayload;
}


std::string QuadSerialPacket::ToString()
{
    std::string result;

    result = HeaderToString();
    result += PayloadToString();


    return result;
}

std::string QuadSerialPacket::HeaderToString()
{
    std::string result;
    uint8_t address = GetAddress();
    if (static_cast< std::size_t >(address) < mAddressStrings.size())
    {
        result += "[" + mAddressStrings[address] + "]";
    }
    else
    {
        result += "[Unknown address]: " + std::to_string(address) + " With control: " + std::to_string(GetControl());
        return result;
    }

    int8_t status = GetControl();
    std::vector<std::string> string;

    switch (address)
    {
    case addresses::Parameters:
        string = mParamControl;
        break;
    case addresses::Log:
        break;
    case addresses::FunctionCall:
        break;
    case addresses::Status:
        string = mStatusControl;
        break;
    case addresses::Debug:
        string = mDebugControl;
        break;
    }
    if (static_cast< std::size_t >(status) < string.size())
    {
        result += "[" + string[status] + "]";
    }
    else
    {
        result += "[Unknown control]: " + std::to_string(status);
    }
    result += "[" + std::to_string(GetIsResend()) + "]";
    return result;
}

std::string QuadSerialPacket::PayloadToString()
{
    std::string result;
    std::string str = mPayload->toString(mHeaderSize);
    result += "[" +  str + "]";
    return result;
}

QuadSerialPacket::QuadSerialPacket(const uint8_t* data, uint16_t data_length, uint8_t offset)
{
    //Assumes data is payload.
    mPayload = QspPayloadRaw::Create(data, data_length, mHeaderSize + offset);
    SetPayloadLength(data_length + offset);
}

QuadSerialPacket::QuadSerialPacket(const uint8_t* data, uint16_t length)
{
    //Assumes data is payload.
    mPayload = QspPayloadRaw::Create(data, length, mHeaderSize);
    SetPayloadLength(length);
}

QuadSerialPacket::QuadSerialPacket(const QspPayloadRaw::Ptr packet):
            mPayload(packet)
{

}

void QuadSerialPacket::SetPayloadLength(uint16_t length)
{
    SerializeInt16(static_cast<int32_t>(length), 2);
}


bool QuadSerialPacket::SerializeInt8(int8_t value, uint32_t start)
{
    if((start) >= mPayload->getPayloadLength())
    {
        return false;
    }
    (*mPayload)[start] = value;
    return true;
}

bool QuadSerialPacket::SerializeInt16(int16_t value, uint32_t start)
{
    if((start + 2) >= mPayload->getPayloadLength())
    {
        return false;
    }
    (*mPayload)[start] = static_cast<uint8_t>(value >> 8);
    (*mPayload)[start + 1] = static_cast<uint8_t>(value);
    return true;
}

bool QuadSerialPacket::SerializeInt32(int32_t value, uint32_t start)
{
    if((start + 4) >= mPayload->getPayloadLength())
    {
        return false;
    }
    (*mPayload)[start]     = static_cast<uint8_t>(value >> 24);
    (*mPayload)[start + 1] = static_cast<uint8_t>(value >> 16);
    (*mPayload)[start + 2] = static_cast<uint8_t>(value >> 8);
    (*mPayload)[start + 3] = static_cast<uint8_t>(value);
    return true;
}


int8_t QuadSerialPacket::DeserializeInt8(uint32_t start)
{
    if((start) >= mPayload->getPayloadLength())
    {
        return 0;
    }
    uint8_t value = 0;
    value = (*mPayload)[start + 0];
    return value;
}

int16_t QuadSerialPacket::DeserializeInt16(uint32_t start)
{
    if((start + 2) >= mPayload->getPayloadLength())
    {
        return 0;
    }
    int value = 0;
    value |= (*mPayload)[start] << 8;
    value |= (*mPayload)[start + 1] ;
    return static_cast<int16_t>(value);
}

int32_t QuadSerialPacket::DeserializeInt32(uint32_t start)
{
  if((start + 4) >= mPayload->getPayloadLength())
  {
      return 0;
  }
  int value = 0;

  value |= ((*mPayload)[start]     << 24);
  value |= ((*mPayload)[start + 1] << 16);
  value |= ((*mPayload)[start + 2] << 8);
  value |= ((*mPayload)[start + 3] );
  return static_cast<int32_t>(value);
}



} /* namespace QuadGS */
