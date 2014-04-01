/*
 * mk_esc.c
 *
 * Copyright (C) 2014  Martin Lundh
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


#include "mk_esc.h"

static mk_esc_motors_t motors;

uint8_t mk_esc_init_motor_control(uint32_t nr_motors)
{

    
    uint8_t chip_address = 0x29;
    int i;
    for (i = 0; i < nr_motors; i++)
    {
       
		if ((i % 2))
        {
            motors.motor[i].twi = twi_0;
            motors.motor[i].twi_notification_semaphore = twi_0_notification_semaphore;
        }
        else
        {
        	motors.motor[i].twi = twi_1;
        	motors.motor[i].twi_notification_semaphore = twi_1_notification_semaphore;
        }
		motors.motor[i].twi_data.buffer = &(motors.motor[i].setpoint_transmitt);
		motors.motor[i].verified = 0;
		motors.motor[i].twi_data.addr[0] = 0;
		motors.motor[i].twi_data.addr_length = 0;
		motors.motor[i].twi_data.length = 2;
		motors.motor[i].twi_data.chip = chip_address;
		motors.motor[i].xtransmit_block_time = BLOCK_TIME_TRANSMIT;
		motors.motor[i].setpoint_transmitt[0] = 0;
		motors.motor[i].setpoint_transmitt[1] = 0;
        
        
        chip_address ++;
        
    }

    return 0;
}


uint8_t mk_esc_update_setpoint(int32_t setpoint[], uint32_t nr_motors)
{
    int i;
    //for (i = 0; i < number_of_motors; i++)
	for (i = nr_motors-1; i >= 0; i--)
    {
        mk_esc_scale_11_bit_setpoint(setpoint[i], motors.motor[i].setpoint_transmitt);
        portBASE_TYPE twi_ans;
		
		// Ensure that the peripheral is available.
		if (xSemaphoreTake(motors.motor[i].twi_notification_semaphore,
				motors.motor[i].xtransmit_block_time) == pdPASS)
		{
			twi_ans = freertos_twi_write_packet_async(motors.motor[i].twi,
				&(motors.motor[i].twi_data),
				motors.motor[i].xtransmit_block_time,
				motors.motor[i].twi_notification_semaphore);
		}		
		else
		{
			twi_ans = ERR_TIMEOUT;
			//ERROR TODO
		}

        
        if (twi_ans != STATUS_OK)
        {
            //for (;;)
            {
                //Error! 
            }
        
        }

    }    
	return 0;
};


void mk_esc_scale_11_bit_setpoint(int32_t data, uint8_t test[2])
{
    if (data < 0)
    {
        data = 0;
    }
    else if (data > 1700)
    {
        data = 1700;
    }

    test[0] = ((data & 0x7f8)>>3);
    test[1] = (data & 0x7);
    if (data<10)
    {
        test[0] = 0;
        test[1] = 0;
    }
}

void mk_esc_enable(uint32_t nr_motors)
{
	int foo = 0;
}
void mk_esc_dissable()
{
	int foo = 0;
}

