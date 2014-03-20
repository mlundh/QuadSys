/*
 * communication_uilitys.h
 *
 * Created: 2013-11-18 20:06:02
 *  Author: Martin Lundh
 */ 


#ifndef COMMUNICATION_UILITYS_H_
#define COMMUNICATION_UILITYS_H_

#include "main.h"







uint8_t decode_QSP_frame(communicaion_packet_t *packet, uint8_t* temp_frame);
uint8_t encode_QSP_frame(communicaion_packet_t *packet, uint8_t* temp_frame);



#endif /* COMMUNICATION_UILITYS_H_ */