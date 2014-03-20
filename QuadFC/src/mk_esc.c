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

#include "asf.h"
#include "mk_esc.h"


int mk_esc_init(mk_esc_motors_t *motors, const uint8_t number_of_motors)
{
    freertos_peripheral_options_t async_driver_options = {
        NULL,											                        /* This peripheral does not need a receive buffer, so this parameter is just set to NULL. */
        0,												                        /* There is no Rx buffer, so the rx buffer size is not used. */
        (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1),	                        /* The priority used by the TWI interrupts. It is essential that the priority does not have a numerically lower value than configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY*/
        TWI_I2C_MASTER,									                        /* TWI is configured as an I2C master. */
        0,																		/* The asynchronous driver is used, so WAIT_TX_COMPLETE and WAIT_RX_COMPLETE are not set. */
    };
    
	freertos_peripheral_options_t async_driver_options1 = {
		NULL,											                        /* This peripheral does not need a receive buffer, so this parameter is just set to NULL. */
		0,												                        /* There is no Rx buffer, so the rx buffer size is not used. */
		(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY),	                        /* The priority used by the TWI interrupts. It is essential that the priority does not have a numerically lower value than configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY*/
		TWI_I2C_MASTER,									                        /* TWI is configured as an I2C master. */
		0,																		/* The asynchronous driver is used, so WAIT_TX_COMPLETE and WAIT_RX_COMPLETE are not set. */
	};
	
	
    freertos_twi_if twi_0 = freertos_twi_master_init(TWI_BUS_0, &async_driver_options);		/*Initialize twi bus 0, available at pin xx and xx on the Arduino due*/
    freertos_twi_if twi_1 = freertos_twi_master_init(TWI_BUS_1, &async_driver_options1);		/*Initialize twi bus 1, available at pin xx and xx on the Arduino due*/

    if (twi_0 == NULL || twi_1 == NULL)
    {
        for (;;)
        {
            // ERROR, TODO set error flag and halt execution*/
        }
    }
    
    /*Create notification semaphores for twi instances.*/
    xSemaphoreHandle twi_0_notification_semaphore;
    xSemaphoreHandle twi_1_notification_semaphore;
    vSemaphoreCreateBinary(twi_0_notification_semaphore);
    vSemaphoreCreateBinary(twi_1_notification_semaphore);
    
    /*Make sure that the semaphores were created successfully.*/
    if (!twi_1_notification_semaphore || !twi_0_notification_semaphore)
    {
        for (;;)
        {
            // ERROR, TODO set error flag and halt execution*/
        }
    }
    

    twi_set_speed(TWI_BUS_0, 400000, sysclk_get_cpu_hz());                       /*High speed TWI setting*/
    twi_set_speed(TWI_BUS_1, 400000, sysclk_get_cpu_hz());                       /*High speed TWI setting*/
    
    uint8_t chip_address = 0x29;
    int i;
    for (i = 0; i < number_of_motors; i++)
    {
       
		if ((i % 2))
        {
            motors->motor[i].twi = twi_0;
            motors->motor[i].twi_notification_semaphore = twi_0_notification_semaphore;      
        }
        else
        {
            motors->motor[i].twi = twi_1;
            motors->motor[i].twi_notification_semaphore = twi_1_notification_semaphore;            
        }
        motors->motor[i].twi_data.buffer = &(motors->motor[i].setpoint_transmitt);
        motors->motor[i].verified = 0;
        motors->motor[i].twi_data.addr[0] = 0;
        motors->motor[i].twi_data.addr_length = 0;
        motors->motor[i].twi_data.length = 2;
        motors->motor[i].twi_data.chip = chip_address;
        motors->motor[i].xtransmit_block_time = BLOCK_TIME_TRANSMIT;
        motors->motor[i].setpoint = 0;
        motors->motor[i].setpoint_transmitt[0] = 0;
        motors->motor[i].setpoint_transmitt[1] = 0;
        
        
        chip_address ++;
        
    }

    return 0;
}


int mk_esc_write_setpoint_value(mk_esc_motors_t *motors, const uint8_t number_of_motors)
{
    int i;
    //for (i = 0; i < number_of_motors; i++)
	for (i = number_of_motors-1; i >= 0; i--)
    {
        mk_esc_scale_11_bit_setpoint(motors->motor[i].setpoint, motors->motor[i].setpoint_transmitt);
        portBASE_TYPE twi_ans;
		
		// Ensure that the peripheral is available.
		if (xSemaphoreTake(motors->motor[i].twi_notification_semaphore,
				motors->motor[i].xtransmit_block_time) == pdPASS)
		{
			twi_ans = freertos_twi_write_packet_async(motors->motor[i].twi,
				&(motors->motor[i].twi_data),
				motors->motor[i].xtransmit_block_time,
				motors->motor[i].twi_notification_semaphore);
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
