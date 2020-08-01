/*
 * slip_packet.h
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
#ifndef SLIP_PACKET_H_
#define SLIP_PACKET_H_

#include <stdint.h>
#include "Components/CharCircularBuffer/inc/CharCircularBuffer.h"

enum Slip_Control_Octets
{
  frame_boundary_octet = 0x7E,
  frame_boundary_octet_replacement = 0x5E,
  control_escape_octet = 0x7D,
  control_escape_octet_replacement = 0x5D
};

enum SLIP_Status
{
  SLIP_StatusNok = 0,
  SLIP_StatusOK = 1,
  SLIP_StatusCont = 2,
};
typedef enum SLIP_Status SLIP_Status_t;

typedef struct SLIP
{
  uint8_t *payload;
  uint16_t packetSize;
  uint16_t allocatedSize;
} SLIP_t;

typedef struct SLIP_Parser
{
  CharCircBuffer_t *cBuffer;
  uint32_t startFound;
  uint32_t index;
  uint8_t  controlEscapeFound;
} SLIP_Parser_t;

/**
 * Create a new SLIP packet. Function will allocate "size" memory and return a initialized package.
 *
 * @param size  Size of array to be allocated on heap.
 * @return      pointer to the new packet.
 */
SLIP_t* Slip_Create(uint16_t size);

/**
 * Delete the SLIP package.
 * @param obj
 */
void Slip_Delete(SLIP_t* obj);

/**
 * Packetize buffer into a SLIP format, copy content of buffer to
 * SLIP packet. The slip packet will also contain a crc field.
 *
 * Will return the length of the slip packet if successful, 0 if fail.
 *
 * @param buffer        Buffer containing the data to be packetized.
 * @param dataLength    Length of the valid data in the buffer.
 * @param bufferLength  Length of buffer, used for boundary check, must be 2 bytes larger than the data length.
 * @param packet        SLIP packet that will be populated.
 * @param addCrc        Set to 1 if packet should be protected by crc.
 * @return              Nr bytes written into packet, 0 if fail.
 */
uint16_t Slip_Packetize(uint8_t* buffer, uint16_t dataLength, uint16_t bufferLength, SLIP_t *packet);

/**
 * @brief Generate the slip parser.
 * 
 * @param internaBuffSize size of the internal buffer.
 * @return SLIP_Parser_t* 
 */
SLIP_Parser_t *SlipParser_Create(uint32_t internaBuffSize);

/**
 * Delete the SLIP parser.
 * @param obj
 */
void SlipParser_Delete(SLIP_Parser_t* obj);

/**
 * Parser that builds a SLIP package via multiple calls. The
 * parser function will return with 0 if no packet
 * is finished, and 1 if there is a package. Any
 * other response is an error condition.
 *
 * The index is used internally to keep track of index in the
 * raw slip packet. User has to provide this to ensure thread
 * safety. User should not update except for discarding data
 * already saved in the SLIP packet.
 * @param buffer          Buffer containing > 1 bytes of data.
 * @param buffer_length   number of bytes in buffer.
 * @param SLIP_packet     Slip packet used internally.
 * @return                1 when done, 0 if not done.
 */
SLIP_Status_t SlipParser_Parse(SLIP_Parser_t* obj, uint8_t *inputBuffer, int InputBufferLength,
                          SLIP_t *SLIP_packet);

#endif /* SLIP_PACKET_H_ */
