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
#include "cubeInit.h"
#include "stm32f4xx_hal_i2c.h"
#include "cubeInit.h"
#include "QuadFC/QuadFC_Peripherals.h"
#include "HMI/inc/led_control_task.h"
#include <string.h>

typedef struct titan_i2c_control {
  I2C_HandleTypeDef     i2c;
  uint32_t              initialized;
  SemaphoreHandle_t     trans_comp_sem;
} titan_i2c_control_t;

const static uint8_t num_i2c = 1;

static  titan_i2c_control_t   titanI2c[] = {
  {{0}, 0, NULL}
};
uint32_t TitanI2c_FindIndex(I2C_HandleTypeDef* i2cHandle);
void TitanI2c_TransCompleteCB(I2C_HandleTypeDef* i2cHandle);
void TitanI2c_ErrorCB(I2C_HandleTypeDef* i2cHandle);
void TitanI2c_AbortCB(I2C_HandleTypeDef* i2cHandle);
void TitanI2c_MSPInitCB(I2C_HandleTypeDef* i2cHandle);


uint8_t QuadFC_i2cInit(int index, int speed)
{
  if(index > num_i2c)
  {
    return 0;
  }
  titanI2c[index].i2c.Instance = I2C1;
  titanI2c[index].i2c.Init.ClockSpeed = speed;
  titanI2c[index].i2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
  titanI2c[index].i2c.Init.OwnAddress1 = 0;
  titanI2c[index].i2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  titanI2c[index].i2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  titanI2c[index].i2c.Init.OwnAddress2 = 0;
  titanI2c[index].i2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  titanI2c[index].i2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  titanI2c[index].trans_comp_sem = xSemaphoreCreateBinary();
  titanI2c[index].initialized = 1;
  HAL_I2C_RegisterCallback(&titanI2c[index].i2c, HAL_I2C_MSPINIT_CB_ID, TitanI2c_MSPInitCB);

  if (HAL_I2C_Init(&titanI2c[index].i2c) != HAL_OK)
  {
    return 0;
  }
  HAL_I2C_RegisterCallback(&titanI2c[index].i2c, HAL_I2C_MASTER_TX_COMPLETE_CB_ID, TitanI2c_TransCompleteCB);
  HAL_I2C_RegisterCallback(&titanI2c[index].i2c, HAL_I2C_MASTER_RX_COMPLETE_CB_ID, TitanI2c_TransCompleteCB);
  HAL_I2C_RegisterCallback(&titanI2c[index].i2c, HAL_I2C_MEM_TX_COMPLETE_CB_ID, TitanI2c_TransCompleteCB);
  HAL_I2C_RegisterCallback(&titanI2c[index].i2c, HAL_I2C_MEM_RX_COMPLETE_CB_ID, TitanI2c_TransCompleteCB);
  HAL_I2C_RegisterCallback(&titanI2c[index].i2c, HAL_I2C_ERROR_CB_ID, TitanI2c_ErrorCB);
  HAL_I2C_RegisterCallback(&titanI2c[index].i2c, HAL_I2C_ABORT_CB_ID, TitanI2c_AbortCB);

  return 1;
}


uint8_t QuadFC_i2cWrite(QuadFC_I2C_t *i2c_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  if(!titanI2c[busIndex].initialized)
  {
    if(!QuadFC_i2cInit(busIndex, 400000))
    {
      return 0;
    }
  }
  while (HAL_I2C_GetState(&titanI2c[busIndex].i2c) != HAL_I2C_STATE_READY)
  {
  
  }
  HAL_StatusTypeDef result = 0;
  if(i2c_data->internalAddrLength > 0)
  {
    result = HAL_I2C_Mem_Write_IT(&titanI2c[busIndex].i2c, (uint16_t)i2c_data->slaveAddress<<1, *i2c_data->internalAddr, i2c_data->internalAddrLength, i2c_data->buffer, i2c_data->bufferLength);
    //result = HAL_I2C_Master_Transmit_IT(&titanI2c[busIndex].i2c, (uint16_t)i2c_data->slaveAddress<<1, i2c_data->internalAddr, i2c_data->internalAddrLength);
    if(result == HAL_OK)
    {
      if(xSemaphoreTake(titanI2c[busIndex].trans_comp_sem, pdMS_TO_TICKS(blockTimeMs)) != pdTRUE)
      {
        return 0;
      }
    }
    else
    {
      return 0;
    }
    
  }
  else
  {
    result &= HAL_I2C_Master_Transmit_IT(&titanI2c[busIndex].i2c, (uint16_t)i2c_data->slaveAddress<<1, i2c_data->buffer, i2c_data->bufferLength);
    if(result == HAL_OK)
    {
      if(xSemaphoreTake(titanI2c[busIndex].trans_comp_sem, pdMS_TO_TICKS(blockTimeMs)) != pdTRUE)
      {
        return 0;
      }
    }
    else
    {
      return 0;
    }
  return 1;
  }
  return 1;

}

uint8_t QuadFC_i2cRead(QuadFC_I2C_t *i2c_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  if(!titanI2c[busIndex].initialized)
  {
    if(!QuadFC_i2cInit(busIndex, 400000))
    {
      return 0;
    }
  }
  while (HAL_I2C_GetState(&titanI2c[busIndex].i2c) != HAL_I2C_STATE_READY)
  {
  
  }
  HAL_StatusTypeDef result = 0;
  if(i2c_data->internalAddrLength > 0) // todo...
  {
//    result = HAL_I2C_Master_Transmit_IT(&titanI2c[busIndex].i2c, (uint16_t)i2c_data->slaveAddress<<1, i2c_data->internalAddr, i2c_data->internalAddrLength);
    result = HAL_I2C_Mem_Read_IT(&titanI2c[busIndex].i2c, (uint16_t)i2c_data->slaveAddress<<1, *i2c_data->internalAddr, i2c_data->internalAddrLength, i2c_data->buffer, i2c_data->bufferLength);
    if(result == HAL_OK)
    {
      if(xSemaphoreTake(titanI2c[busIndex].trans_comp_sem, pdMS_TO_TICKS(blockTimeMs)) != pdTRUE)
      {
        return 0;
      }
    }
    else
    {
      return 0;
    }
    
  }
  else
  {
    result &= HAL_I2C_Master_Receive_IT(&titanI2c[busIndex].i2c, (uint16_t)i2c_data->slaveAddress<<1, i2c_data->buffer, i2c_data->bufferLength);
    if(result == HAL_OK)
    {
      if(xSemaphoreTake(titanI2c[busIndex].trans_comp_sem, pdMS_TO_TICKS(blockTimeMs)) != pdTRUE)
      {
        return 0;
      }
    }
    else
    {
      return 0;
    }
  }
  return 1;
}

uint32_t TitanI2c_FindIndex(I2C_HandleTypeDef* i2cHandle)
{
  uint32_t index = 0;
  for(uint32_t i = 0; i < num_i2c; i++)
  {
    if(&titanI2c[i].i2c == i2cHandle)
    {
      index = i;
      break;
    }
  }
  return index;
}


void TitanI2c_TransCompleteCB(I2C_HandleTypeDef* i2cHandle)
{
  portBASE_TYPE higherPriorityTaskHasWoken = pdFALSE;

  uint32_t busIndex = TitanI2c_FindIndex(i2cHandle);
  xSemaphoreGiveFromISR(titanI2c[0].trans_comp_sem, &higherPriorityTaskHasWoken);

  if(higherPriorityTaskHasWoken == pdTRUE)
  {
    portEND_SWITCHING_ISR(higherPriorityTaskHasWoken);
  }
}

void TitanI2c_ErrorCB(I2C_HandleTypeDef* i2cHandle)
{

}
void TitanI2c_AbortCB(I2C_HandleTypeDef* i2cHandle)
{

}
void TitanI2c_MSPInitCB(I2C_HandleTypeDef* i2cHandle)
{
  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {  
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration    
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA 
    */
    GPIO_InitStruct.Pin = I2C_SCL_IMU_Pin|I2C_SDA_IMU_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    /* I2C1 interrupt Init */
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
  }
}

void I2C1_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&titanI2c[0].i2c);
}

/**
  * @brief This function handles I2C1 error interrupt.
  */
void I2C1_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(&titanI2c[0].i2c);
}