/*
 * prepare_output.h
 *
 * Created: 2013-07-15 17:29:29
 *  Author: Martin Lundh
 */

#ifndef PREPARE_OUTPUT_H_
#define PREPARE_OUTPUT_H_

#include "stdint.h"
#include "stddef.h"

uint8_t my_itoa( int32_t val, uint8_t *out_str );
int32_t my_atoi( uint8_t *, uint8_t );

#endif /* PREPARE_OUTPUT_H_ */
