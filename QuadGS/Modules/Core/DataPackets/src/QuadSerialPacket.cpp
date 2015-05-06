/*
 * QuadSerialPacket.cpp
 *
 *  Created on: Feb 14, 2015
 *      Author: martin
 */

#include "QuadSerialPacket.h"
#include <stdlib.h>
namespace QuadGS {

QuadSerialPacket::QuadSerialPacket(uint8_t* data, uint8_t length)
{
    mPayload = QspPayloadRaw::Create(data, length);
}

QuadSerialPacket::QuadSerialPacket(const QspPayloadRaw::Ptr packet):
            mPayload(packet)
{

}

QuadSerialPacket::~QuadSerialPacket()
{
}

uint8_t QuadSerialPacket::GetAddress()
{
    return (*mPayload)[0];
}
void QuadSerialPacket::SetAddress(uint8_t address)
{
    (*mPayload)[0] = address;
}

uint8_t QuadSerialPacket::GetControl()
{
    return (*mPayload)[1];
}
void QuadSerialPacket::SetControl(uint8_t control)
{
    (*mPayload)[1] = control;
}
bool QuadSerialPacket::SetPayload(QspPayloadRaw::Ptr pl)
{
    if(!mPayload && pl)
        {
            mPayload = pl;
        }
    else
    {
        return false;
    }
    return true;
}

QspPayloadRaw::Ptr QuadSerialPacket::GetPayload()
{
    return  mPayload;
}

bool QuadSerialPacket::SerializeInt8(int8_t value, uint start)
{
    if((start) >= mPayload->getPayloadLength())
    {
        return false;
    }
    (*mPayload)[start] = value;
    return true;
}

bool QuadSerialPacket::SerializeInt16(int16_t value, uint start)
{
    if((start + 2) >= mPayload->getPayloadLength())
    {
        return false;
    }
    (*mPayload)[start] = value >> 8;
    (*mPayload)[start + 1] = value;
    return true;
}

bool QuadSerialPacket::SerializeInt32(int32_t value, uint start)
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


uint8_t QuadSerialPacket::DeserializeInt8(uint start)
{
    if((start) >= mPayload->getPayloadLength())
    {
        return 0;
    }
    uint8_t value = 0;
    value = (*mPayload)[start + 0];
    return value;
}

uint16_t QuadSerialPacket::DeserializeInt16(uint start)
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

uint32_t QuadSerialPacket::DeserializeInt32(uint start)
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
