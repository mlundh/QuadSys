/*
 * slip_packet.h
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
#ifndef SLIP_PACKET_H_
#define SLIP_PACKET_H_

#include "stdint.h"

enum Slip_Control_Octets
{
  frame_boundary_octet = 0x7E,
  frame_boundary_octet_replacement = 0x5E,
  control_escape_octet = 0x7D,
  control_escape_octet_replacement = 0x5D
};

typedef struct SLIP
{
  uint8_t *payload;
  uint16_t packetSize;
  uint16_t allocatedSize;
} SLIP_t;

/**
 * Create a new SLIP packet. Function will allocate "size" memory and return a initialized package.
 *
 * @param size  Size of array to be allocated on heap.
 * @return      pointer to the new packet.
 */
SLIP_t *Slip_Create(uint16_t size);


/**
 * Packetize buffer into a SLIP format, copy content of buffer to
 * SLIP packet.
 *
 * Will return the length of the slip packet if successful, 0 if fail.
 *
 * @param buffer        Buffer containing the data to be packetized.
 * @param bufferLength  Length of buffer, used for boundary check.
 * @param packet        SLIP packet that will be populated.
 * @return              Nr bytes written into packet, 0 if fail.
 */
uint16_t Slip_Packetize(uint8_t* buffer, uint16_t bufferLength, SLIP_t *packet);


/**
 * Unpack the data in packet, and copy it to buffer.
 *
 * Will return number of bytes copied into buffer, 0 if fail.
 *
 * @param buffer        Buffer that will be populated with data from the packet.
 * @param bufferLength  Length of buffer, used for boundary check.
 * @param packet        SLIP packet containing the data.
 * @return              Number of bytes copied from packet to buffer. 0 if fail.
 */
uint16_t Slip_DePacketize(uint8_t* buffer, uint16_t bufferLength, SLIP_t *packet);



#endif /* SLIP_PACKET_H_ */
