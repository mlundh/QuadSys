/*
 * Test_task.h
 *
 * Created: 2013-02-09 14:01:38
 *  Author: Martin Lundh
 *
 *  Data is stored according to:
 *
 *  [start byte][address][control][length][(re-)start byte][data (0-MAX_DATA_LENGTH bytes)][crc][end byte]
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
uint8_t Com_HandleQSP(QSP_t *packet, QSP_t *QSP_RspPacket);
uint8_t Com_HandleParameters(QSP_t *QSP_packet, QSP_t *QSP_RspPacket);
uint8_t Com_HandleStatus(QSP_t *QSP_packet, QSP_t *QSP_RspPacket);

void Com_SendResponse(QSP_t *QSP_RspPacket);

uint8_t Com_AddCRC( QSP_t *packet);
uint8_t Com_CheckCRC( QSP_t *packet);



#endif /* COMMUNICATION-TASKS-H- */
