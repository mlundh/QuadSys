/*
 * SlipPacket.cpp
 *
 *  Created on: Feb 21, 2015
 *      Author: martin
 */

#include "SlipPacket.h"

namespace QuadGS {


SlipPacket::SlipPacket(QspPayloadRaw::Ptr Payload):
        mPayload(Payload)
{
    mPacket = QspPayloadRaw::Create(255);
    Encode();
}

SlipPacket::SlipPacket(uint8_t* Payload, uint8_t PayloadLength)

{
    mPacket  = QspPayloadRaw::Create(Payload, PayloadLength);
    mPayload = QspPayloadRaw::Create(255);
    Decode();
}

SlipPacket::~SlipPacket()
{
}

SlipPacket::SlipPacketPtr SlipPacket::Create(uint8_t* Payload, uint8_t PayloadLength)
{
    if((!Payload) && !(PayloadLength > 0) && !(PayloadLength < 255))
    {
        return SlipPacket::SlipPacketPtr();
    }
    if((Payload[0] == frame_boundary_octet) && (Payload[PayloadLength - 1] == frame_boundary_octet))
    {
        //ensure it is a valid SLIP packet and then create a new object.
        SlipPacketPtr p( new SlipPacket( Payload, PayloadLength )  );
        return p;

    }
    else
    {
        return SlipPacket::SlipPacketPtr();
    }
}
SlipPacket::SlipPacketPtr SlipPacket::Create(QspPayloadRaw::Ptr Payload)
{
    if(Payload)
    {
        //ensure it is a valid SLIP packet and then create a new object.
        SlipPacketPtr p( new SlipPacket( Payload ) );
        return p;

    }
    else
    {
        return SlipPacket::SlipPacketPtr();
    }
}

QspPayloadRaw::Ptr SlipPacket::GetPayload()
{
    return mPayload;
}
QspPayloadRaw::Ptr SlipPacket::GetPacket()
{
    return mPacket;
}

// TODO check and add control!
bool SlipPacket::Encode()
{
    (*mPacket)[0] = SlipControlOctets::frame_boundary_octet;

    std::size_t k;
  std::size_t i;
  int nrEscapeOctets = 0;
  for ( i = 0, k = 1; i < (mPayload->getPayloadLength()); i++, k++ )
  {
    if ( (*mPayload)[i] == frame_boundary_octet )
    {
        (*mPacket)[k++] = control_escape_octet;
        (*mPacket)[k] = frame_boundary_octet_replacement;
      nrEscapeOctets++;
    }
    else if ( (*mPayload)[i] == control_escape_octet )
    {
        (*mPacket)[k++] = control_escape_octet;
        (*mPacket)[k] = control_escape_octet_replacement;
      nrEscapeOctets++;
    }
    else
    {
        (*mPacket)[k] = (*mPayload)[i];
    }
  }
  (*mPacket)[k] = frame_boundary_octet;
  mPacket->setPayloadLength(++k);
  return true;
}

bool SlipPacket::Decode()
{
    std::size_t k;
  std::size_t i;
  if((*mPacket)[0] != SlipControlOctets::frame_boundary_octet)
  {
    return false; // Todo, error!
  }
  for ( i = 0, k = 1; k < (mPacket->getPayloadLength() - 1); i++, k++ )
  {
    if ( (*mPacket)[k] == control_escape_octet )
    {
      k++;
      if ( (*mPacket)[k] == control_escape_octet_replacement )
      {
          (*mPayload)[i] = control_escape_octet;
      }
      else if ( (*mPacket)[k] == frame_boundary_octet_replacement )
      {
          (*mPayload)[i] = frame_boundary_octet;
      }
      else
      {
        return false; //Todo, error!
      }
    }
    else
    {
        (*mPayload)[i] = (*mPacket)[k];
    }
  }
  mPayload->setPayloadLength(++i);
  return true;
}



} /* namespace QuadGS */
