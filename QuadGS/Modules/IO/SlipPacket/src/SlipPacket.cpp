/*
 * SlipPacket.cpp
 *
 *  Created on: Feb 21, 2015
 *      Author: martin
 */

#include "SlipPacket.h"
#include <iostream>
#include <sstream>
#include <string>

extern "C"
{
  #include "crc.h"
}

namespace QuadGS {

bool SlipPacket::mCrcInit(false);

SlipPacket::SlipPacket(QspPayloadRaw::Ptr data, bool isPayload)
{
  initCrc();
  if(isPayload)
  {
    mPayload = data;
    mPacket = QspPayloadRaw::Create(255);
    Encode();
  }
  else
  {
    mPacket = data;
    mPayload = QspPayloadRaw::Create(255);
    Decode();
  }

}

SlipPacket::SlipPacket(const uint8_t* data, uint8_t dataLength, bool isPayload)

{
  initCrc();
  if(isPayload)
  {
    mPayload  = QspPayloadRaw::Create(data, dataLength);
    mPacket = QspPayloadRaw::Create(255);
    Encode();
  }
  else
  {
    mPacket  = QspPayloadRaw::Create(data, dataLength);
    mPayload = QspPayloadRaw::Create(255);
    Decode();
  }
}

SlipPacket::~SlipPacket()
{
}


void SlipPacket::initCrc()
{
  if(!mCrcInit)
  {
    crcInit();
    mCrcInit = true;
  }
}
SlipPacket::SlipPacketPtr SlipPacket::Create(const uint8_t* data, uint8_t dataLength, bool isPayload)
{
    if((!data) && !(dataLength > 0) && !(dataLength < 255))
    {
        throw std::runtime_error("Payload length out of reach.");
    }
    if((data[0] == frame_boundary_octet) && (data[dataLength - 1] == frame_boundary_octet))
    {
        //ensure it is a valid SLIP packet and then create a new object.
        SlipPacketPtr p( new SlipPacket( data, dataLength, isPayload )  );
        return p;

    }
    else
    {
        throw std::runtime_error("Not a valid slip packet.");
    }
}
SlipPacket::SlipPacketPtr SlipPacket::Create(QspPayloadRaw::Ptr data, bool isPayload)
{
    if(data)
    {
      SlipPacketPtr p( new SlipPacket( data, isPayload ) );
      return p;
    }
    else
    {
      throw std::runtime_error("Trying to create empty slip packet.");
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
  
  addChecksumToPayload();
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
  (*mPacket)[k++] = frame_boundary_octet;
  
  mPacket->setPayloadLength(k);
  return true;
}

bool SlipPacket::Decode()
{
  std::size_t k;
  std::size_t i;
  if((*mPacket)[0] != SlipControlOctets::frame_boundary_octet)
  {
    throw std::runtime_error("Not a valid Slip Packet");
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
        throw std::runtime_error("Not a valid Slip Packet");
      }
    }
    else
    {
        (*mPayload)[i] = (*mPacket)[k];
    }
  }
  mPayload->setPayloadLength(i);
  verifyChecksum();
  return true;
}

void SlipPacket::addChecksumToPayload()
{
  // Add CRC16 checksum to the payload.
  std::size_t plLength = mPayload->getPayloadLength();
  uint16_t msg_crc = crcFast(mPayload->getPayload(), plLength);
  (*mPayload)[plLength++] = (msg_crc >> 8);
  (*mPayload)[plLength++] = (msg_crc);
  mPayload->setPayloadLength(plLength);
}
void SlipPacket::verifyChecksum()
{
  std::size_t plLength = mPayload->getPayloadLength();
  uint16_t calc_crc = crcFast(mPayload->getPayload(), plLength - 2 );
  
  uint16_t msg_crc = 0;
  msg_crc |= (*mPayload)[plLength - 2] << 8;
  msg_crc |=  (*mPayload)[plLength - 1];
  mPayload->setPayloadLength(plLength - 2); // remove CRC, it is not a part of the QSP.
  if(msg_crc != calc_crc)
  {
    std::stringstream ss;
    ss <<  "calc crc: " << std::hex << calc_crc << ", msg crc: " << std::hex << msg_crc << ".";
    throw std::runtime_error(ss.str());
  }
}

} /* namespace QuadGS */
