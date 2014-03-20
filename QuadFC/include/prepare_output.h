/*
 * prepare_output.h
 *
 * Created: 2013-07-15 17:29:29
 *  Author: Martin Lundh
 */ 


#ifndef PREPARE_OUTPUT_H_
#define PREPARE_OUTPUT_H_
#include "main.h"


uint8_t my_atoi(int32_t val, uint8_t **out_str);
void reverse(uint8_t *str, int32_t length);
uint8_t end_string_print(uint8_t **str);
uint8_t PrepareControlValuesPidSend(uint8_t **str, control_values_pid_t *pidValues);



#endif /* PREPARE_OUTPUT_H_ */