/**********************************************************************
 *
 * Filename:    crc.h
 * 
 * Description: A header file describing the various CRC standards.
 *
 * Notes:       
 *
 * 
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/
/*Modified by Martin Lundh to fit QuadFC*/
#ifndef CRC_H_
#define CRC_H_
#include <stdint.h>
/*
 * CRC16 standard from the list that follows.
 */

#define CRC_NAME			"CRC-CCITT"
#define POLYNOMIAL			0x1021
#define INITIAL_REMAINDER	0xFFFF
#define FINAL_XOR_VALUE		0x0000
#define CHECK_VALUE			0x29B1

/*Crc lookup table.*/
extern uint16_t *crcTable;

void crcInit( void );
uint16_t crcFast( uint8_t const *message, int nBytes );

#endif /* CRC_H_ */
