/*
 * prepare_output.c
 *
 * Created: 2013-07-15 17:29:16
 *  Author: Martin Lundh
 */ 

#include "prepare_output.h"

/* Returns the string representation of a 32bit signed integer. 
 * The function assumes that the uint8_t pointer has enough
 * space allocated to represent the whole representation.
 */
uint8_t my_atoi(int32_t val, uint8_t **out_str)
{
	// if negative, need 1 byte for the sign
	int sign = val < 0? 1: 0;
    uint8_t i = 0;

    //if the number is 0 then return '0'
    if (val == 0) 
    {
        *(*out_str + i++) = '0';
    }
    else if (val < 0)
    {
        *(*out_str + i++) = '-';
        val = -val;
    }
	while (val > 0) {
    	*(*out_str + i++) = '0' + val % 10;
    	val /= 10;
	}

	// reverse order on all digits, sign is already in the right place.
	reverse(*out_str + sign, i - sign);
    *out_str = (*out_str + i);
    return i;
}



void reverse(uint8_t *str, int32_t length)
{
	int i = 0, j = length-1;
	uint8_t tmp;
	while (i < j) {
		tmp = str[i];
		str[i] = str[j];
		str[j] = tmp;
		i++; j--;
	}
}

uint8_t end_string_print(uint8_t **str)
{
    uint8_t i = 0;
    *(*str + i++) = '\r';
    *(*str + i++) = '\n';
    *(*str + i++) = '\0';
    *str = *str + i;
    return i;
}

uint8_t PrepareControlValuesPidSend(uint8_t **str, control_values_pid_t *pidValues)
{
    uint8_t i = 0;
    uint8_t unit_size = (sizeof(int32_t));
    int32_t **temp_str = (int32_t **)str;
    
    *(*temp_str + i++) = pidValues->pitch_p;
    *(*temp_str + i++) = pidValues->pitch_i;
    *(*temp_str + i++) = pidValues->pitch_d;
    *(*temp_str + i++) = pidValues->roll_p;
    *(*temp_str + i++) = pidValues->roll_i;
    *(*temp_str + i++) = pidValues->roll_d;
    *(*temp_str + i++) = pidValues->yaw_p;
    *(*temp_str + i++) = pidValues->yaw_i;
    *(*temp_str + i++) = pidValues->yaw_d;
    *(*temp_str + i++) = pidValues->altitude_p;
    *(*temp_str + i++) = pidValues->altitude_i;
    *(*temp_str + i++) = pidValues->altitude_d;
    
    *str = (*str + unit_size * i) ;
    return unit_size * i;   
}