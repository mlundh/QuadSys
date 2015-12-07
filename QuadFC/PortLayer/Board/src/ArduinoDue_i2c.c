/*
 * Board_Peripherals.c
 *
 * Copyright (C) 2015 martin
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
#include "QuadFC_Peripherals.h"
#include "freertos_twi_master.h"
#include "led_control_task.h"
#include "string.h"


static  Twi    *        twi_instances[] = {TWI0, TWI1};
static  freertos_twi_if freeRTOS_twi[]  = {NULL, NULL};
static  uint32_t        twi_Init[]      = {0, 0};
static  twi_package_t   tmp_twi;


uint8_t Init_Twi(int index)
{
  if(index > (sizeof(twi_instances)/sizeof(twi_instances[0])))
  {
    return 0;
  }
  if(twi_Init[index] == 1)
  {
    return 1; // allready initialized.
  }
  freertos_peripheral_options_t async_driver_options = {
      NULL,
      0,
      (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1),
      TWI_I2C_MASTER,
      (USE_TX_ACCESS_MUTEX | USE_RX_ACCESS_MUTEX | WAIT_TX_COMPLETE | WAIT_RX_COMPLETE),
  };

  /*Initialize twi bus.*/
  freertos_twi_if twi = freertos_twi_master_init( twi_instances[index],
      &async_driver_options );


  if ( twi == NULL )
  {
    Led_Set(led_error_TWI);
    return 0;
  }
  freeRTOS_twi[index] = twi;
  twi_set_speed( twi_instances[index], 400000, sysclk_get_cpu_hz() ); /*High speed TWI setting*/
  twi_Init[index] = 1;
  return 1;
}


uint8_t QuadFC_i2cWrite(QuadFC_I2C_t *i2c_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  if(busIndex > (sizeof(twi_instances)/sizeof(twi_instances[0])))
  {
    Led_Set(led_error_TWI);
    return 0;
  }
  if(!twi_Init[busIndex])
  {
    Led_Set(led_error_TWI);
    if(!Init_Twi(busIndex))
    {
      return 0;
    }
  }
  TickType_t blockTimeTicks = (blockTimeMs / portTICK_PERIOD_MS);

  status_code_t twi_status = 0;
  tmp_twi.addr_length = i2c_data->internalAddrLength;
  memcpy(tmp_twi.addr, i2c_data->internalAddr, i2c_data->internalAddrLength);
  tmp_twi.buffer = i2c_data->buffer;
  tmp_twi.length = i2c_data->bufferLength;
  tmp_twi.chip = i2c_data->slaveAddress;

  twi_status = freertos_twi_write_packet( freeRTOS_twi[busIndex],
      &tmp_twi,
      blockTimeTicks );


  if (twi_status != STATUS_OK)
  {
    Led_Set(led_error_TWI);
    return 0;
  }
  return 1;
}

uint8_t QuadFC_i2cRead(QuadFC_I2C_t *i2c_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  if(busIndex > (sizeof(twi_instances)/sizeof(twi_instances[0])))
  {
    Led_Set(led_error_TWI);
    return 0;
  }
  if(!twi_Init[busIndex])
  {
    if(!Init_Twi(busIndex))
    {
      Led_Set(led_error_TWI);
      return 0;
    }
  }
  TickType_t blockTimeTicks = (blockTimeMs / portTICK_PERIOD_MS);

  status_code_t twi_status = 0;
  tmp_twi.addr_length = i2c_data->internalAddrLength;
  memcpy(tmp_twi.addr, i2c_data->internalAddr, i2c_data->internalAddrLength);
  tmp_twi.buffer = i2c_data->buffer;
  tmp_twi.length = i2c_data->bufferLength;
  tmp_twi.chip = i2c_data->slaveAddress;

  twi_status = freertos_twi_read_packet( freeRTOS_twi[busIndex],
      &tmp_twi,
      blockTimeTicks );

  if (twi_status != STATUS_OK)
  {
    Led_Set(led_error_TWI);
    return 0;
  }
  return 1;
}
