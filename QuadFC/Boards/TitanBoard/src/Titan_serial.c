/*
 * ArduinoDue_serial.c
 *
 * Copyright (C) 2015 Martin Lundh
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


#include "string.h"
#include "QuadFC/QuadFC_Peripherals.h"
#include "HMI/inc/led_control_task.h"
#include "usart.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f413xx.h"



typedef struct titan_uart_control {
  USART_TypeDef*      usart;
  uint32_t            initialized;
  UART_HandleTypeDef  uart_handle;
  SemaphoreHandle_t   rx_trans_comp_sem;
  SemaphoreHandle_t   tx_trans_comp_sem;

} titan_uart_control_t;

const static uint8_t num_uart = 7;

static  titan_uart_control_t   uart[] = {
  {UART10, 0, {0}, NULL, NULL}, // RC 1
  {USART1, 0, {0}, NULL, NULL}, // Com serial USB
  {USART2, 0, {0}, NULL, NULL},
  {UART9,  0, {0}, NULL, NULL}, // Applog serial backend
  {USART3, 0, {0}, NULL, NULL},
  {USART6, 0, {0}, NULL, NULL},
  {UART8,  0, {0}, NULL, NULL},

};

#define RECEIVE_BUFFER_LENGTH (512)

/**
 * @brief Local uart handler, called by all uart peripherals.
 * 
 * @param usart_index 
 */

static uint32_t Parity[] =
{
    UART_PARITY_NONE,
    UART_PARITY_ODD,
    UART_PARITY_EVEN
};

static uint32_t StopBits[] =
{
    UART_STOPBITS_1,
    UART_STOPBITS_1,
    UART_STOPBITS_2
};

static uint32_t WordLength[] =
{
    0,
    0,
    0,
    UART_WORDLENGTH_8B,
    UART_WORDLENGTH_9B
};


int TitanSerial_GetUartIndex(UART_HandleTypeDef *huart);

void TitanSerial_TxCompHandler(UART_HandleTypeDef *huart);
void TitanSerial_RxCompHandler(UART_HandleTypeDef *huart);
void TitanSerial_AbortTxCpltHandler(UART_HandleTypeDef *huart);
void TitanSerial_AbortRxCpltHandler(UART_HandleTypeDef *huart);
void TitanSerial_ErrorHandler(UART_HandleTypeDef *huart);

void TitanUART_GPIOInit(UART_HandleTypeDef* uartHandle);
void TitanUART_GPIODeInit(UART_HandleTypeDef* uartHandle);


uint8_t QuadFC_SerialInit(int busIndex, QuadFC_SerialOptions_t* opt)
{
  if(busIndex > (num_uart) || uart[busIndex].initialized)
  {
    return 0;
  }
 
  HAL_UART_RegisterCallback(&(uart[busIndex].uart_handle), HAL_UART_MSPINIT_CB_ID, TitanUART_GPIOInit);
  HAL_UART_RegisterCallback(&(uart[busIndex].uart_handle), HAL_UART_MSPDEINIT_CB_ID, TitanUART_GPIODeInit);

  uart[busIndex].uart_handle.Instance = uart[busIndex].usart;
  uart[busIndex].uart_handle.Init.BaudRate = opt->baudRate;
  uart[busIndex].uart_handle.Init.WordLength = WordLength[opt->dataBits];
  uart[busIndex].uart_handle.Init.StopBits = StopBits[opt->stopBits];
  uart[busIndex].uart_handle.Init.Parity = Parity[opt->parityType];
  uart[busIndex].uart_handle.Init.Mode = UART_MODE_TX_RX;
  uart[busIndex].uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  uart[busIndex].uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;
  uart[busIndex].rx_trans_comp_sem = xSemaphoreCreateCounting(portMAX_DELAY, 0);
  uart[busIndex].tx_trans_comp_sem = xSemaphoreCreateCounting(portMAX_DELAY, 0);

  if (HAL_UART_Init(&uart[busIndex].uart_handle) != HAL_OK)
  {
    return 0;
  }

  HAL_UART_RegisterCallback(&(uart[busIndex].uart_handle), HAL_UART_TX_COMPLETE_CB_ID, TitanSerial_TxCompHandler);
  HAL_UART_RegisterCallback(&(uart[busIndex].uart_handle), HAL_UART_RX_COMPLETE_CB_ID, TitanSerial_RxCompHandler);
  uart[busIndex].initialized= 1;
  return 1;
}


uint8_t QuadFC_SerialWrite(QuadFC_Serial_t *serial_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  if(busIndex > (num_uart) || !uart[busIndex].initialized)
  {
    return 0;
  }

  TickType_t blockTimeTicks = (blockTimeMs / portTICK_PERIOD_MS);

  // Make sure that the semaphore is in an expected state. 
  xSemaphoreTake( uart[busIndex].tx_trans_comp_sem, ( TickType_t ) 0 );

  // TODO this is a blocking call! Use irq and/or dma! 
  HAL_StatusTypeDef result = HAL_UART_Transmit_IT(&uart[busIndex].uart_handle, serial_data->buffer, serial_data->bufferLength);
  if(result != HAL_OK)
  {
    return 0;
  }
  if( xSemaphoreTake( uart[busIndex].tx_trans_comp_sem, ( TickType_t ) blockTimeTicks ) == pdTRUE )
  {
    return 1;
  }
  else
  {
    return 0;
  }
  
  return 1;
}
uint32_t QuadFC_SerialRead(QuadFC_Serial_t *serial_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  if(busIndex > (num_uart) || !uart[busIndex].initialized)
  {
    return 0;
  }

  TickType_t blockTimeTicks;
  if(blockTimeMs == portMAX_DELAY)
  {
    blockTimeTicks = portMAX_DELAY;
  }
  else
  {
    blockTimeTicks= (blockTimeMs / portTICK_PERIOD_MS);
  }
  //HAL_UART_Receive_IT();
  vTaskDelay(1);
  return 0;
}



int TitanSerial_GetUartIndex(UART_HandleTypeDef *huart)
{
  for(int i = 0; i < num_uart; i++)
  {
    if(&uart[i].uart_handle == huart)
    {
      return i;
    }
  }
  return -1;
}


void TitanSerial_TxCompHandler(UART_HandleTypeDef *huart)
{
  int busIndex = TitanSerial_GetUartIndex(huart);

  if(busIndex < 0)
  {
    return;
  }

  portBASE_TYPE higher_priority_task_woken = pdFALSE;
	xSemaphoreGiveFromISR(uart[busIndex].tx_trans_comp_sem, &higher_priority_task_woken);
  /* If giving the semaphore triggered a higher-priority task to be unblocked
  then this call will ensure that the isr returns to the task with higer priority.
  
  Usually this should be placed at the very end of an ISR, but for a cortex M4 port this is ok.*/
	portEND_SWITCHING_ISR(higher_priority_task_woken);
}
void TitanSerial_RxCompHandler(UART_HandleTypeDef *huart)
{

}
void TitanSerial_AbortTxCpltHandler(UART_HandleTypeDef *huart)
{

}
void TitanSerial_AbortRxCpltHandler(UART_HandleTypeDef *huart)
{

}
void TitanSerial_ErrorHandler(UART_HandleTypeDef *huart)
{

}


void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&uart[1].uart_handle);
}

void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&uart[2].uart_handle);
}

void USART3_IRQHandler(void)
{
  HAL_UART_IRQHandler(&uart[4].uart_handle);
}

void USART6_IRQHandler(void)
{
  HAL_UART_IRQHandler(&uart[5].uart_handle);
}

void UART8_IRQHandler(void)
{
  HAL_UART_IRQHandler(&uart[6].uart_handle);
}

void UART9_IRQHandler(void)
{
  HAL_UART_IRQHandler(&uart[3].uart_handle);
}

void UART10_IRQHandler(void)
{
  HAL_UART_IRQHandler(&uart[0].uart_handle);
}

void TitanUART_GPIOInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==UART8)
  {
    __HAL_RCC_UART8_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**UART8 GPIO Configuration    
    PE0     ------> UART8_RX
    PE1     ------> UART8_TX 
    */
    GPIO_InitStruct.Pin = UART_MOTOR_1_RX_Pin|UART_MOTOR_1_RXE1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART8;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* UART8 interrupt Init */
    HAL_NVIC_SetPriority(UART8_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART8_IRQn);
  }
  else if(uartHandle->Instance==UART9)
  {
    __HAL_RCC_UART9_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**UART9 GPIO Configuration    
    PD14     ------> UART9_RX
    PD15     ------> UART9_TX 
    */
    GPIO_InitStruct.Pin = UART_DBG_RX_Pin|UART_DBG_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_UART9;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* UART9 interrupt Init */
    HAL_NVIC_SetPriority(UART9_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART9_IRQn);
  }
  else if(uartHandle->Instance==UART10)
  {
    __HAL_RCC_UART10_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**UART10 GPIO Configuration    
    PE2     ------> UART10_RX
    PE3     ------> UART10_TX 
    */
    GPIO_InitStruct.Pin = UART_RC_1_RX_Pin|UART_RC_1_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_UART10;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* UART10 interrupt Init */
    HAL_NVIC_SetPriority(UART10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART10_IRQn);
  }
  else if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_ENABLE();
  
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration    
    PA10     ------> USART1_RX
    PA11     ------> USART1_CTS
    PA12     ------> USART1_RTS
    PA15     ------> USART1_TX 
    */
    GPIO_InitStruct.Pin = UART_COM_USB_RX_Pin|UART_COM_USB_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART_COM_USB_CTS_Pin|UART_COM_USB_RTS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
  else if(uartHandle->Instance==USART2)
  {
    __HAL_RCC_USART2_CLK_ENABLE();
  
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART2 GPIO Configuration    
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = UART_RC_2_TX_Pin|UART_RC_2_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  }
  else if(uartHandle->Instance==USART3)
  {
    __HAL_RCC_USART3_CLK_ENABLE();
  
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration    
    PC5     ------> USART3_RX
    PB14     ------> USART3_RTS
    PD8     ------> USART3_TX
    PD11     ------> USART3_CTS 
    */
    GPIO_InitStruct.Pin = UART_COM_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(UART_COM_RX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART_COM_RTS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(UART_COM_RTS_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART_COM_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(UART_COM_TX_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART_COM_CTS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(UART_COM_CTS_GPIO_Port, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  }
  else if(uartHandle->Instance==USART6)
  {
    __HAL_RCC_USART6_CLK_ENABLE();
  
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART6 GPIO Configuration    
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX 
    */
    GPIO_InitStruct.Pin = UART_MOTOR_2_TX_Pin|UART_MOTOR_2_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART6 interrupt Init */
    HAL_NVIC_SetPriority(USART6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);
  }
}

void TitanUART_GPIODeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART8)
  {
    __HAL_RCC_UART8_CLK_DISABLE();
  
    /**UART8 GPIO Configuration    
    PE0     ------> UART8_RX
    PE1     ------> UART8_TX 
    */
    HAL_GPIO_DeInit(GPIOE, UART_MOTOR_1_RX_Pin|UART_MOTOR_1_RXE1_Pin);
    HAL_NVIC_DisableIRQ(UART8_IRQn);
  }
  else if(uartHandle->Instance==UART9)
  {
    __HAL_RCC_UART9_CLK_DISABLE();
  
    /**UART9 GPIO Configuration    
    PD14     ------> UART9_RX
    PD15     ------> UART9_TX 
    */
    HAL_GPIO_DeInit(GPIOD, UART_DBG_RX_Pin|UART_DBG_TX_Pin);
    HAL_NVIC_DisableIRQ(UART9_IRQn);
  }
  else if(uartHandle->Instance==UART10)
  {
    __HAL_RCC_UART10_CLK_DISABLE();
  
    /**UART10 GPIO Configuration    
    PE2     ------> UART10_RX
    PE3     ------> UART10_TX 
    */
    HAL_GPIO_DeInit(GPIOE, UART_RC_1_RX_Pin|UART_RC_1_TX_Pin);
    HAL_NVIC_DisableIRQ(UART10_IRQn);
  }
  else if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA10     ------> USART1_RX
    PA11     ------> USART1_CTS
    PA12     ------> USART1_RTS
    PA15     ------> USART1_TX 
    */
    HAL_GPIO_DeInit(GPIOA, UART_COM_USB_RX_Pin|UART_COM_USB_CTS_Pin|UART_COM_USB_RTS_Pin|UART_COM_USB_TX_Pin);
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  }
  else if(uartHandle->Instance==USART2)
  {
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(GPIOD, UART_RC_2_TX_Pin|UART_RC_2_RX_Pin);
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  }
  else if(uartHandle->Instance==USART3)
  {
    __HAL_RCC_USART3_CLK_DISABLE();
  
    /**USART3 GPIO Configuration    
    PC5     ------> USART3_RX
    PB14     ------> USART3_RTS
    PD8     ------> USART3_TX
    PD11     ------> USART3_CTS 
    */
    HAL_GPIO_DeInit(UART_COM_RX_GPIO_Port, UART_COM_RX_Pin);
    HAL_GPIO_DeInit(UART_COM_RTS_GPIO_Port, UART_COM_RTS_Pin);
    HAL_GPIO_DeInit(GPIOD, UART_COM_TX_Pin|UART_COM_CTS_Pin);

    HAL_NVIC_DisableIRQ(USART3_IRQn);
  }
  else if(uartHandle->Instance==USART6)
  {
    __HAL_RCC_USART6_CLK_DISABLE();
  
    /**USART6 GPIO Configuration    
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX 
    */
    HAL_GPIO_DeInit(GPIOC, UART_MOTOR_2_TX_Pin|UART_MOTOR_2_RX_Pin);
    HAL_NVIC_DisableIRQ(USART6_IRQn);
  }
} 
