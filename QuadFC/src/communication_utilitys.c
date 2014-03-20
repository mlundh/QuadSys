/*
 * communication_utilitys.c
 *
 *
 * Copyright (C) 2014  Martin Lundh
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 *
 * Packet format:
 * [start][address][control][information][crc][stop]
 *
 *
 *
 *
 */ 
#include "communication_uilitys.h"



uint8_t encode_QSP_frame(communicaion_packet_t *packet, uint8_t* temp_frame)
{
	temp_frame[0] = packet->address;
	temp_frame[1] = packet->control;
	temp_frame[2] = packet->length;
	int i;
	for (i = 0; i < packet->length; i++)
	{
		temp_frame[3 + i] = packet->information[i];
	}
	//Calculate and append CRC16 checksum.
	uint16_t crc_check;
	crc_check = crcFast(temp_frame, packet->length + 3);
	packet->crc[0] = (uint8_t)((crc_check >> 0) & 0Xff);
	packet->crc[1] = (uint8_t)((crc_check >> 8) & 0Xff);
	
	temp_frame[3 + i++] = packet->crc[0];
	temp_frame[3 + i] = packet->crc[1];


	// Create the real packet, changing reserved bit patterns if
	// necessary.
	packet->frame[0] = frame_boundary_octet;
	int k;
	int nrEscapeOctets = 0;
	for (i = 0, k = 1; i < (packet->length + NR_OVERHEAD_BYTES - 2); i++, k++)
	{
		if (temp_frame[i] == frame_boundary_octet)
		{
			packet->frame[k++] = control_escape_octet;
			packet->frame[k] = frame_boundary_octet_replacement;
			nrEscapeOctets++;
		}
		else if (temp_frame[i] == control_escape_octet)
		{
			packet->frame[k++] = control_escape_octet;
			packet->frame[k] = control_escape_octet_replacement;
			nrEscapeOctets++;
		}
		else
		{
			packet->frame[k] = temp_frame[i];
		}
	}
	packet->frame[k] = frame_boundary_octet;
	packet->frame_length = packet->length + NR_OVERHEAD_BYTES + nrEscapeOctets;
	return ++k;
}


uint8_t decode_QSP_frame(communicaion_packet_t *packet, uint8_t* temp_frame)
{
	int k;
	int i;

	for (i = 0, k = 1; k < packet->frame_length; i++, k++)
	{
		if (packet->frame[k] == control_escape_octet)
		{
			k++;
			if (packet->frame[k] == control_escape_octet_replacement)
			{
				temp_frame[i] = control_escape_octet;
			}
			else if (packet->frame[k] == frame_boundary_octet_replacement)
			{
				temp_frame[i] = frame_boundary_octet;
			}
			else
			{
				// TODO better exeption handling/throwing
			}
		}
		else
		{
			temp_frame[i] = packet->frame[k];
		}

	}

	packet->address = temp_frame[0];
	packet->control = temp_frame[1];
	packet->length = temp_frame[2];

	packet->crc[0] = temp_frame[3 + packet->length];
	packet->crc[1] = temp_frame[3 + packet->length + 1];
	for (i = 0; i < packet->length; i++)
	{
		packet->information[i] = temp_frame[3 + i];
	}
	//Calculate and compare CRC16 checksum.
	uint16_t crc_check;
	uint8_t crc[2];
	
	crc_check = crcFast(temp_frame, packet->length + 3);
	crc[0] = (uint8_t)((crc_check >> 0) & 0Xff);
	crc[1] = (uint8_t)((crc_check >> 8) & 0Xff);
	if ((packet->crc[0] != crc[0]) || (packet->crc[1] != crc[1]))
	{
		packet->control = ctrl_crc_error;
		return -1;
	}
	return 0;
}
