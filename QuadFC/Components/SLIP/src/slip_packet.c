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
#include "Components/SLIP/inc/slip_packet.h"
#include "Components/CRC16/inc/crc.h"
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
    if(!package || !package->payload) // TODO one malloc...
    {
        return NULL;
    }
    package->allocatedSize = size;
    package->packetSize = 0;
    return package;
}

void Slip_Delete(SLIP_t* obj)
{
    if(!obj)
    {
        return;
    }
    vPortFree(obj->payload);
    vPortFree(obj);
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


uint8_t SLIP_AddCRC(uint8_t *buffer, uint16_t *dataLength, uint16_t bufferLength)
{
    if (bufferLength < (*dataLength + 2))
    {
        return 0;
    }
    //Calculate and append CRC16 checksum.
    uint16_t crcCalc = crcFast(buffer, *dataLength);
    buffer[*dataLength] = (uint8_t)((crcCalc >> 8) & 0Xff);
    buffer[*dataLength + 1] = (uint8_t)((crcCalc)&0Xff);
    *dataLength += 2;
    return 1;
}

uint8_t SLIP_CheckCRC(uint8_t *buffer, uint16_t *dataLength)
{
    //Calculate and compare CRC16 checksum.
    uint16_t crcCalc = 0;
    uint16_t crcMsg = 0;
    crcCalc = crcFast(buffer, *dataLength - 2);

    crcMsg |= (uint16_t)(buffer[*dataLength - 2]) << 8;
    crcMsg |= (uint16_t)(buffer[*dataLength - 1]);
    *dataLength -= 2;
    if (crcCalc != crcMsg)
    {
        return 0;
    }
    return 1;
}

SLIP_Parser_t *SlipParser_Create(uint32_t internaBuffSize)
{
    SLIP_Parser_t *parser = pvPortMalloc(sizeof(SLIP_Parser_t));
    parser->cBuffer = CharCircBuff_Create(internaBuffSize);
    parser->index = 0; 
    parser->startFound = 0;
    parser->controlEscapeFound = 0;
    return parser;
}

void SlipParser_Delete(SLIP_Parser_t *obj)
{
    CharCircBuff_Delete(obj->cBuffer);
    vPortFree(obj);
}

void SlipParser_Reset(SLIP_Parser_t* obj)
{
    obj->startFound = 0;
    obj->index = 0;
    obj->controlEscapeFound = 0;
}

SLIP_Status_t SlipParser_Parse(SLIP_Parser_t* obj, uint8_t *inputBuffer, uint32_t InputBufferLength,
                          SLIP_t *SLIP_packet)
{
    uint32_t packetComplete = 0;
    if(!SLIP_packet || !obj)
    {
        return SLIP_StatusNok;
    }
    uint32_t nrBuffered = CharCircBuff_NrElemets(obj->cBuffer);
    SLIP_Status_t result = SLIP_StatusCont;
    while(InputBufferLength || nrBuffered)
    {
        uint8_t dataFromFifo = 0;
        uint8_t* buffer = NULL;
        uint32_t bufferLength = 0;

        if(nrBuffered) // First handle the buffered chars. 
        {
            CharCircBuff_Pop(obj->cBuffer,&dataFromFifo,1);
            buffer = &dataFromFifo;
            bufferLength = 1;
            nrBuffered--;
        }
        else if (InputBufferLength) // Then the new chars.
        {
            buffer = inputBuffer;
            bufferLength = InputBufferLength;
            InputBufferLength = 0; // We will handle all of the input buffer. If there is an excess it will be pushed to the cbuff.
        }
        else // Continue.
        {
            return SLIP_StatusCont;
        }

        if ((bufferLength > (SLIP_packet->allocatedSize - obj->index)))
        {
            SlipParser_Reset(obj);
            return SLIP_StatusLengthError;
        }

        for (int i = 0; i < bufferLength; i++)
        {
            if(obj->controlEscapeFound) 
            {
                obj->controlEscapeFound = 0;
                if ( buffer[i] == control_escape_octet_replacement )
                {
                    SLIP_packet->payload[obj->index++] = control_escape_octet;
                }
                else if ( buffer[i] == frame_boundary_octet_replacement )
                {
                    SLIP_packet->payload[obj->index++] = frame_boundary_octet;
                }
                else
                {
                    SlipParser_Reset(obj);
                    return SLIP_StatusEscapeError;
                }
            }
            else if (buffer[i] == frame_boundary_octet)
            {
                if (obj->startFound && obj->index > 4) // Last boundary of frame. A frame should have more than 4 bytes. This ensures we are not miss-aligned. 
                {
                    SLIP_packet->packetSize = obj->index;
                    obj->index = 0;
                    result = SLIP_StatusOK;
                    obj->startFound = 0;
                    obj->controlEscapeFound = 0;
                    if(!SLIP_CheckCRC(SLIP_packet->payload, &SLIP_packet->packetSize))
                    {
                        SlipParser_Reset(obj);
                        return SLIP_StatusCrcError;
                    }
                    packetComplete = 1;
                    
                    //Finished packet! return.
                }
                else // First boundary of frame.
                {
                    obj->index = 0;
                    obj->startFound = 1;
                }
            }// Frame boundary
            else if (buffer[i] == control_escape_octet ) // Escape char
            {
                obj->controlEscapeFound = 1;
            }
            else
            {
                SLIP_packet->payload[obj->index++] = buffer[i];
            }
            if(packetComplete) // if the packet is complete, then we must store the rest of the data in the circular buffer.
            {
                CharCircBuff_Push(obj->cBuffer, &buffer[i], (bufferLength - i));
                break;
            }
        }//for()
    }
    
    return result;
}

