/*
 * QuadSerialPacket.cpp
 *
 *  Created on: Feb 14, 2015
 *      Author: martin
 */

#include "QuadSerialPacket.h"
#include <stdlib.h>
namespace QuadGS {

const uint8_t QuadSerialPacket::mHeaderSize(4);

QuadSerialPacket::Ptr QuadSerialPacket::Create(const uint8_t* data, uint8_t length)
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
    return DeserializeInt8(1);
}
void QuadSerialPacket::SetControl(uint8_t control)
{
    SerializeInt8(control, 1);
}

uint16_t QuadSerialPacket::GetPayloadLength()
{
    return DeserializeInt16(2);
}

QspPayloadRaw::Ptr QuadSerialPacket::GetPayload()
{
    // Copy payload to new QspPayloadRaw object.
    return QspPayloadRaw::Create(mPayload->getPayload() + (sizeof(uint8_t)*mHeaderSize), GetPayloadLength());

}

QspPayloadRaw::Ptr QuadSerialPacket::GetRawData()
{
    return  mPayload;
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
    (*mPayload)[start] = value >> 8;
    (*mPayload)[start + 1] = value;
    return true;
}

bool QuadSerialPacket::SerializeInt32(int32_t value, uint32_t start)
{
    if((start + 4) >= mPayload->getPayloadLength())
    {
        return false;
    }
    (*mPayload)[start] = value >> 24;
    (*mPayload)[start + 1] = value >> 16;
    (*mPayload)[start + 2] = value >> 8;
    (*mPayload)[start + 3] = value;
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
    int16_t value = 0;
    value |= (*mPayload)[start] << 8;
    value |= (*mPayload)[start + 1] ;
    return value;
}

int32_t QuadSerialPacket::DeserializeInt32(uint32_t start)
{
  if((start + 4) >= mPayload->getPayloadLength())
  {
      return 0;
  }
  int32_t value = 0;

  value |= (*mPayload)[start]     << 24;
  value |= (*mPayload)[start + 1] << 16;
  value |= (*mPayload)[start + 2] << 8;
  value |= (*mPayload)[start + 3] ;
    return value;
}



} /* namespace QuadGS */
