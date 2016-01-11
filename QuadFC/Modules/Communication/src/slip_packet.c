/*
 * slip_packet.c
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

#include <stddef.h>
#include "Communication/inc/slip_packet.h"
#include "Communication/inc/crc.h"
#include "FreeRTOS.h"

/**
 * Add crc to the buffer before packing into the SLIP package. This is a
 * non standard part of the SLIP package, but it serves it purpose here.
 * @param buffer        Pointer to the buffer that should have a crc appended.
 * @param dataLength    Length of data in the buffer. Will get updated by the function.
 * @param bufferLength  Length of the buffer itself. Has to be at least 2 bytes longer than the dataLength.
 * @return              1 if success, 0 otherwise.
 */
uint8_t SLIP_AddCRC(uint8_t* buffer, uint16_t *dataLength, uint16_t bufferLength);

/**
 * Check crc of the buffer. The crc is assumed to be located at the end of the buffer, the last two
 * positions.
 * @param buffer        Pointer to the buffer.
 * @param dataLength    Length of data in the buffer. Will get updated by the function.
 * @return              1 if success, 0 otherwise.
 */
uint8_t SLIP_CheckCRC(uint8_t* buffer, uint16_t *dataLength);


SLIP_t* Slip_Create(uint16_t size)
{
  if(size < 2)
  {
    return NULL;
  }
  SLIP_t *package = pvPortMalloc( sizeof(SLIP_t) );
  package->payload = pvPortMalloc(sizeof(uint8_t) * size);
  if(!package || !package->payload)
  {
    return NULL;
  }
  package->allocatedSize = size;
  package->packetSize = 0;
  return package;
}

uint16_t Slip_Packetize(uint8_t* buffer, uint16_t dataLength, uint16_t BufferLength, SLIP_t *packet)
{
  if((dataLength + 4) > packet->allocatedSize) // packet must be at least as long as buffer + start and stop bytes and crc.
  {
    return 0;
  }
  // Add crc.
  if(!SLIP_AddCRC(buffer, &dataLength, BufferLength))
  {
    return 0;
  }

  packet->payload[0] = frame_boundary_octet;
  int k;
  int i;
  for ( i = 0, k = 1; i < (dataLength); i++, k++ )
  {
    if((k + 2) > packet->allocatedSize) // make sure that this iteration will fit in packet.
    {
      return 0;
    }

    if ( buffer[i] == frame_boundary_octet )
    {
      packet->payload[k++] = control_escape_octet;
      packet->payload[k] = frame_boundary_octet_replacement;
    }
    else if ( buffer[i] == control_escape_octet )
    {
      packet->payload[k++] = control_escape_octet;
      packet->payload[k] = control_escape_octet_replacement;
    }
    else
    {
      packet->payload[k] = buffer[i];
    }
  }
  packet->payload[k] = frame_boundary_octet;
  packet->packetSize = ++k;
  return k;
}


uint16_t Slip_DePacketize(uint8_t* buffer, uint16_t bufferLength, SLIP_t *packet)
{
  uint16_t k;
  uint16_t i;
  if((packet->payload[0] != frame_boundary_octet) ||
      (bufferLength < packet->packetSize - 2))
  {
    return 0;
  }
  for ( i = 0, k = 1; k < (packet->packetSize - 1); i++, k++ )
  {
    if ( packet->payload[k] == control_escape_octet )
    {
      k++;
      if ( packet->payload[k] == control_escape_octet_replacement )
      {
        buffer[i] = control_escape_octet;
      }
      else if ( packet->payload[k] == frame_boundary_octet_replacement )
      {
        buffer[i] = frame_boundary_octet;
      }
      else
      {
        return 0;
      }
    }
    else
    {
      buffer[i] = packet->payload[k];
    }
  }
  if(!SLIP_CheckCRC(buffer, &i))
  {
    return 0;
  }
  return i;
}


SLIP_Status_t SLIP_Parser(uint8_t *inputBuffer, int InputBufferLength,
    SLIP_t *SLIP_packet, int *index)
{
  if ( (InputBufferLength < 1) || (InputBufferLength > (SLIP_packet->allocatedSize - *index)))
  {
    return 0;
  }
  SLIP_Status_t result = SLIP_StatusCont;
  for ( int i = 0; i < InputBufferLength; i++, (*index)++ )
  {
    SLIP_packet->payload[*index] = inputBuffer[i];
    if ( SLIP_packet->payload[*index] == frame_boundary_octet )
    {
      if ( *index > 4 ) // Last boundary of frame. Frame minimum length = 4
      {
        SLIP_packet->packetSize = *index + 1;
        *index = 0;
        result = SLIP_StatusOK;
        break;
        //Finished packet! return.
      }
      else // First boundary of frame.
      {
        *index = 0;
        SLIP_packet->payload[*index] = inputBuffer[i];
      }
    }// Frame boundary
  }//for()
  return result;
}

uint8_t SLIP_AddCRC(uint8_t* buffer, uint16_t *dataLength, uint16_t bufferLength)
{
  if(bufferLength < (*dataLength + 2))
  {
    return 0;
  }
  //Calculate and append CRC16 checksum.
  uint16_t crcCalc = crcFast(buffer, *dataLength);
  buffer[*dataLength ] = (uint8_t) ((crcCalc >> 8) & 0Xff);
  buffer[*dataLength + 1] = (uint8_t) ((crcCalc) & 0Xff);
  *dataLength += 2;
  return 1;
}

uint8_t SLIP_CheckCRC(uint8_t* buffer, uint16_t *dataLength)
{
  //Calculate and compare CRC16 checksum.
  uint16_t crcCalc = 0;
  uint16_t crcMsg = 0;
  crcCalc = crcFast( buffer, *dataLength - 2);

  crcMsg |=  (uint16_t)(buffer[*dataLength - 2]) << 8;
  crcMsg |=  (uint16_t)(buffer[*dataLength - 1]);
  *dataLength -= 2;
  if ( crcCalc != crcMsg )
  {
    return 0;
  }
  return 1;
}
