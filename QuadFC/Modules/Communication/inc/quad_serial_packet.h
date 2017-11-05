/*
 * quad_serial_packet.h
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
 * | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *
 *  +-------------------------------+
 * 0|            address            |
 *  +-------------------------------+
 * 1| R|         Control            |
 *  +-------------------------------+
 * 2|         payload size H        |
 *  +-------------------------------+
 * 3|        payload size L         |
 *  +-------------------------------+
 * 4|            payload            |
 *  +-------------------------------+
 *  Where the R field indicates resend.
 *
 * frame format:
 * [address][isResend][control][PayloadSize][Payload]
 *
 *
 * A parameter frame extends the QSP by adding a new
 * static field in the header (first byte in the payload):
 *
 * | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *
 *  +-------------------------------+
 * 0|            address            |
 *  +-------------------------------+
 * 1| R|         Control            |
 *  +-------------------------------+
 * 2|         payload size H        |
 *  +-------------------------------+
 * 3|        payload size L         |
 *  +-------------------------------+
 * 4| L|         payload            |
 *  +-------------------------------+
 * 5|            payload            |
 *  +-------------------------------+
 *  The R field indicates resend.
 *  The L field indicates if it is the last in a sequence.
 *
 */

typedef struct QuadSerialPacket QSP_t;

enum QSP_addresses
{
  QSP_Parameters = 0,
  QSP_Log = 1,
  QSP_FunctionCall = 2,
  QSP_Status = 3,
  QSP_Debug = 4,
  QSP_Transmission = 5,
};
enum QSP_ParametersControl
{
  QSP_ParamSetTree = 0,
  QSP_ParamGetTree = 1,
  QSP_ParamValue = 2,
  QSP_ParamSave = 3,
  QSP_ParamLoad = 4,
};
enum QSP_LogControl
{
  QSP_LogName = 0,
  QSP_LogEntry = 1,
  QSP_LogStopAll = 2,
};
enum QSP_StatusControl
{
  QSP_StatusCont = 0,
  QSP_StatusAck = 1,
  QSP_StatusNack = 2,
  QSP_StatusCrcError = 3,
  QSP_StatusNotAllowed = 4,
  QSP_StatusUnexpectedSequence = 5,
  QSP_StatusNotValidSlipPacket = 6,
  QSP_StatusBufferOverrun = 7,
  QSP_StatusNotImplemented = 200,
};
enum QSP_TransmissionControl
{
  QSP_TransmissionOK = 0,
  QSP_TransmissionNOK = 1,

};
enum QSP_DebugControl
{
  QSP_DebugGetRuntimeStats = 0,
  QSP_DebugSetRuntimeStats = 1,
  QSP_DebugGetErrorMessages = 2,
};

typedef enum QSP_StatusControl QSP_StatusControl_t;

/**
 * Acquire semaphore indicating that the QSP is populated.
 * @param current   Current QSP.
 * @return          1 if successful, 0 otherwise.
 */
uint8_t QSP_TakeIsPopulated(QSP_t *current);

/**
 * Notify that the QSP is populated.
 * @param current   Current QSP.
 * @return          1 if successful, 0 otherwise.
 */
uint8_t QSP_GiveIsPopulated(QSP_t *current);

/**
 * Acquire semaphore indicating that the QSP is empty.
 * @param current   Current QSP.
 * @return          1 if successful, 0 otherwise.
 */
uint8_t QSP_TakeIsEmpty(QSP_t *current);

/**
 * Notify that the QSP is empty.
 * @param current   Current QSP.
 * @return          1 if successful, 0 otherwise.
 */
uint8_t QSP_GiveIsEmpty(QSP_t *current);

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
 * Get the isResend field of the packet.
 * @param current   Current packet, get isResend field from this packet.
 * @return          0 if fail, isResend field otherwise.
 */
uint8_t QSP_GetIsResend(QSP_t *current);

/**
 * Set isResend field of the packet.
 * @param current   Current packet, set isResend field of this packet.
 * @param control   Control to be set.
 * @return          0 if fail, 1 otherwise.
 */
uint8_t QSP_SetIsResend(QSP_t *current, uint8_t resend);

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
uint16_t QSP_GetHeaderdSize(void);




/*-------------------------Param section---------------------------*/

/**
 * Get the payload pointer of the param message. Param messages
 * contains an extra field in the header, therefore the payload
 * is located with an offset of one.
 * @return
 */
uint8_t *QSP_GetParamPayloadPtr(QSP_t *current);

/**
 * Get the sequence number of a param packet.
 * @param current   Current packet.
 * @return          The sequence number of the packet.
 */
uint8_t QSP_GetParamSequenceNumber(QSP_t *current);

/**
 * Set the sequence number of a param packet.
 * @param current         Current packet.
 * @param sequenceNumber  The sequence number to write.
 * @return                0 if fail, 1 otherwise.
 */
uint8_t QSP_SetParamSequenceNumber(QSP_t *current, uint8_t sequenceNumber);

/**
 * Get field indicating if it is the last message in the
 * sequence.
 * @param current       Current packet.
 * @return 0 if not last, 1 otherwise.
 */
uint8_t QSP_GetParamLastInSeq(QSP_t *current);

/**
 * Set if message is the last in sequence.
 * @param current       Current packet.
 * @param isLast
 */
void  QSP_SetParamLastInSeq(QSP_t *current, uint8_t isLast);

/**
 * Get header size of the param packet.
 */
uint16_t QSP_GetParamHeaderdSize(void);

/**
 * Get the payloadSize field of the packet. Param message type. Subtracts one
 * from the value in the QSP due to the extra static field before the
 * payload.
 * @param current   Current packet, get payloadSize field from this packet.
 * @return          0 if fail, payloadSize field otherwise.
 */
uint16_t QSP_GetParamPayloadSize(QSP_t *current);

/**
 * Set payloadSize field of the packet. Param message type. Adds one to the
 * given size to accommodate the extra static field in the param message.
 * @param current       Current packet, set payloadSize field of this packet.
 * @param payloadSize   Control to be set.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t QSP_SetParamPayloadSize(QSP_t *current, uint16_t payloadSize);


#endif /* MODULES_COMMUNICATION_INC_QUAD_SERIAL_PACKET_H_ */
