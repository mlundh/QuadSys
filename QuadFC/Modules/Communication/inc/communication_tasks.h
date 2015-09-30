/*
 * Test_task.h
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


/*  Data is stored according to:
 *
 *  [start byte][address][control][length][data (0-MAX_DATA_LENGTH bytes)][crc][end byte]
 *
 *  and the start byte is escaped according to QSP_control_octets enum.
 *
 *
 */

#ifndef COMMUNICATION_TASKS_H_
#define COMMUNICATION_TASKS_H_

#include "crc.h"
#include "stdint.h"
#include "slip_packet.h"
#include "quad_serial_packet.h"


void Com_CreateTasks( void );

uint8_t Com_SendQSP( QSP_t *packet );
uint8_t Com_HandleQSP(QSP_t *packet, QSP_t *QSP_RspPacket, SLIP_t *SLIP_packet);
uint8_t Com_HandleParameters(QSP_t *QSP_packet, QSP_t *QSP_RspPacket, SLIP_t *SLIP_packet);
uint8_t Com_HandleStatus(QSP_t *QSP_packet, QSP_t *QSP_RspPacket, SLIP_t *SLIP_packet);

void Com_SendResponse(QSP_t *QSP_RspPacket);

uint8_t Com_AddCRC( QSP_t *packet);
uint8_t Com_CheckCRC( QSP_t *packet);



#endif /* COMMUNICATION-TASKS-H- */
