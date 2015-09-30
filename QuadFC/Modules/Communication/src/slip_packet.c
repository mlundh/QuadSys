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

#include "stddef.h"
#include "slip_packet.h"
#include "FreeRTOS.h"



SLIP_t *Slip_Create(uint16_t size)
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

uint16_t Slip_Packetize(uint8_t* buffer, uint16_t bufferLength, SLIP_t *packet)
{
  if((bufferLength + 2) > packet->allocatedSize) // packet must be at least as long as buffer + start and stop bytes.
  {
    return 0;
  }
  packet->payload[0] = frame_boundary_octet;
  int k;
  int i;
  for ( i = 0, k = 1; i < (bufferLength); i++, k++ )
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
  int k;
  int i;
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
  return ++i;
}
