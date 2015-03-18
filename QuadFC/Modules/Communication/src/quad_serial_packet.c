/*
 * quad_serial_packet.c
 *
 * Copyright (C) 2015 martin
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

#include "string.h"
#include "stddef.h"
#include "quad_serial_packet.h"

#define QSP_HEADER_SIZE (4)

struct QuadSerialPacket
{
  uint8_t *payload;
  uint16_t allocatedSize;
  uint16_t extPacketSize;
  SemaphoreHandle_t xMutex;
} ;

QSP_t *QSP_Create(uint16_t size)
{
  if(size < 4)
  {
    return NULL;
  }
  QSP_t *package = pvPortMalloc( sizeof(QSP_t) );
  package->payload = pvPortMalloc(sizeof(uint8_t) * size);
  package->xMutex = xSemaphoreCreateMutex();
  if(!package || !package->payload || !package->xMutex)
  {
    return NULL;
  }
  package->extPacketSize = QSP_HEADER_SIZE;
  package->allocatedSize = size;
  QSP_SetPayloadSize(package, 0);
  return package;
}

uint8_t QSP_TakeOwnership(QSP_t *current)
{
  uint8_t result = 0;
  if( xSemaphoreTake( current->xMutex, (0) ) == pdTRUE )
  {
    result = 1;
  }
  /*Could not obtain mutex.*/
  return result;
}

uint8_t QSP_GiveOwnership(QSP_t *current)
{
  xSemaphoreGive(current->xMutex);
  return 1;
}

uint8_t QSP_GetAddress(QSP_t *current)
{
  return current->payload[0];
}


uint8_t QSP_SetAddress(QSP_t *current, uint8_t address)
{
  current->payload[0] = address;
  return 1;
}

uint8_t QSP_GetControl(QSP_t *current)
{
  return current->payload[1];
}

uint8_t QSP_SetControl(QSP_t *current, uint8_t control)
{
  current->payload[1] = control;
  return 1;
}

uint16_t QSP_GetPayloadSize(QSP_t *current)
{
  int value = 0;
  value |= current->payload[2] << 8;
  value |= current->payload[3] ;
  return ((uint16_t)value);
}

uint8_t QSP_SetPayloadSize(QSP_t *current, uint16_t payloadSize)
{
  if(payloadSize > (current->allocatedSize - QSP_HEADER_SIZE))
  {
    return 0;
  }
  current->payload[2] = (uint8_t)(payloadSize >> 8);
  current->payload[3] = (uint8_t)(payloadSize);
  return 1;
}

uint8_t *QSP_GetPayloadPtr(QSP_t *current)
{
  return (current->payload + QSP_HEADER_SIZE);
}

uint8_t QSP_ClearPayload(QSP_t *current)
{
  QSP_GetPayloadPtr(current)[0] = '\0';
  QSP_SetPayloadSize(current, 0);
  return 0;
}

uint8_t QSP_SetAfterPayload(QSP_t *current, uint8_t *buffer, uint16_t bufferLength)
{
  if(current->allocatedSize < (QSP_GetPayloadSize(current) + QSP_HEADER_SIZE + bufferLength))
  {
    return 1;
  }
  memcpy((current->payload + QSP_HEADER_SIZE + QSP_GetPayloadSize(current)), buffer, bufferLength);
  current->extPacketSize += bufferLength;
  return 0;
}

uint16_t QSP_GetPacketSize(QSP_t *current)
{
  return current->extPacketSize;
}

uint8_t *QSP_GetPacketPtr(QSP_t *current)
{
  return current->payload;
}

uint16_t QSP_GetAvailibleSize(QSP_t *current)
{
  return (current->allocatedSize);
}


uint8_t QSP_SetPayload(QSP_t *current, uint8_t *buffer, uint16_t bufferLength)
{
  if((bufferLength + QSP_HEADER_SIZE) > current->allocatedSize)
  {
    return 0;
  }
  memcpy((current->payload + QSP_HEADER_SIZE), buffer, bufferLength);
  QSP_SetPayloadSize(current, bufferLength);
  current->extPacketSize = bufferLength + QSP_HEADER_SIZE;
  return 1;
}

uint16_t QSP_GetHeaderdSize()
{
  return QSP_HEADER_SIZE;
}
