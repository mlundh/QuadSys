/*
 * quad_serial_packet.h
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
#ifndef MODULES_COMMUNICATION_INC_QUAD_SERIAL_PACKET_H_
#define MODULES_COMMUNICATION_INC_QUAD_SERIAL_PACKET_H_

#include "stdint.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#define QSP_MAX_PACKET_SIZE (256)

/**
 * Big endian, MSB0
 *
 * Struct containing the data protocol.
 * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *
 *  +-------------------------------+
 * 1|            address            |
 *  +-------------------------------+
 * 2|            Control            |
 *  +-------------------------------+
 * 3|          payload size         |
 *  +-------------------------------+
 * 4|           payload             |
 *  +-------------------------------+
 *
 * frame format:
 * [address][control][PayloadSize][Payload]
 */

typedef struct QuadSerialPacket QSP_t;

enum QSP_addresses
{
  QSP_Parameters = 1,
  QSP_Log = 2,
  QSP_FunctionCall = 3,
  QSP_Status = 4, // TODO remove from QSP, should reside a layer beneath.
};
enum QSP_ParametersControl
{
  QSP_ParamSetTree = 2,
  QSP_ParamGetTree = 3,
  QSP_ParamValue = 4,
};
enum QSP_StatusControl
{
  QSP_StatusAck = 1,
  QSP_StatusNack = 2,
  QSP_StatusCrcError = 3,
  QSP_StatusNotAllowed = 4,
  QSP_StatusUnexpectedSequence = 4,
  QSP_StatusNotImplemented = 200,
};
typedef enum QSP_StatusControl QSP_StatusControl_t;

uint8_t QSP_TakeOwnership(QSP_t *current);

uint8_t QSP_GiveOwnership(QSP_t *current);

/**
 * Create a new QSP packet. Function will allocate "size" memory and return a initialized package.
 *
 * @param size  Size of array to be allocated on heap. Has to be > 4.
 * @return      pointer to the new packet. NULL if fail.
 */
QSP_t *QSP_Create(uint16_t size);


/**
 * Get the address field of the packet.
 * @param current   Current packet, get address from this packet.
 * @return          0 if fail, address otherwise.
 */
uint8_t QSP_GetAddress(QSP_t *current);

/**
 * Set address of packet.
 * @param current   Current packet, to set address to.
 * @param address   Address to be set.
 * @return          0 if fail, 1 otherwise.
 */
uint8_t QSP_SetAddress(QSP_t *current, uint8_t address);

/**
 * Get the control field of the packet.
 * @param current   Current packet, get control field from this packet.
 * @return          0 if fail, control field otherwise.
 */
uint8_t QSP_GetControl(QSP_t *current);

/**
 * Set control field of the packet.
 * @param current   Current packet, set control field of this packet.
 * @param control   Control to be set.
 * @return          0 if fail, 1 otherwise.
 */
uint8_t QSP_SetControl(QSP_t *current, uint8_t control);

/**
 * Get the payloadSize field of the packet.
 * @param current   Current packet, get payloadSize field from this packet.
 * @return          0 if fail, payloadSize field otherwise.
 */
uint16_t QSP_GetPayloadSize(QSP_t *current);

/**
 * Set payloadSize field of the packet.
 * @param current       Current packet, set payloadSize field of this packet.
 * @param payloadSize   Control to be set.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t QSP_SetPayloadSize(QSP_t *current, uint16_t payloadSize);

/**
 * Get payload pointer, points into the array of current.
 *
 * @param current   Current packet to get payload of.
 * @return          Pointer to where the payload starts in the message. NULL if fail.
 */
uint8_t *QSP_GetPayloadPtr(QSP_t *current);

/**
 * Clear the content of the payload. Will only set first
 * element to null and set payload length to zero.
 * @param current   Current packet to clear the payload of.
 * @return          0 if fail, 1 otherwise.
 */
uint8_t QSP_ClearPayload(QSP_t *current);


/**
 * Set data after end of payload. This can be used to add data that is not
 * a standard part of the QSP, for example crc.
 * @param current       Current packet.
 * @param buffer        Buffer containing the data to add after payload.
 * @param bufferLength  Length of data to add.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t QSP_SetAfterPayload(QSP_t *current, uint8_t *buffer, uint16_t bufferLength);

/**
 * Get the size of the complete package, including non-standard fields.
 * @param current     Current packet.
 * @return            Length of complete payload in bytes.
 */
uint16_t QSP_GetPacketSize(QSP_t *current);

/**
 * Get packet pointer, points to the beginning of the array of current.
 *
 * @param current   Current packet.
 * @return          Pointer to where the packet starts. NULL if fail.
 */
uint8_t *QSP_GetPacketPtr(QSP_t *current);

/**
 * Get the number of bytes available for payload.
 * @param current   Current packet.
 * @return          Number of available bytes.
 */
uint16_t QSP_GetAvailibleSize(QSP_t *current);

/**
 * Set buffer as payload to current by deep copy.
 *
 * @param current       Current packet to set payload of.
 * @param buffer        Buffer to be set as payload.
 * @param bufferLength  Length of buffer.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t QSP_SetPayload(QSP_t *current, uint8_t *buffer, uint16_t bufferLength);

/**
 * Get header size of the packet.
 */
uint16_t QSP_GetHeaderdSize();

#endif /* MODULES_COMMUNICATION_INC_QUAD_SERIAL_PACKET_H_ */
