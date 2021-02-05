/*
 * TitanTest_serial.c
 *
 * Copyright (C) 2020 Martin Lundh
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
#include "QuadFC/QuadFC_Serial.h"
#include "HMI/inc/led_control_task.h"
#include "usart.h"
#include "stm32f413xx.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"


typedef struct titan_test_uart_control {
  USART_TypeDef*        usart;
  uint32_t              initialized;
  uint8_t*              TxBuffPtr;
} titan_test_uart_control_t;

static  titan_test_uart_control_t   uart = {
  UART9,  0, NULL
};


void TitanTestUART_GPIOInit(USART_TypeDef* usart);

uint8_t Test_SerialInit()
{
  if(uart.initialized)
  {
    return 1;
  }  
  TitanTestUART_GPIOInit(uart.usart);

  LL_USART_InitTypeDef  init = {0};

  init.BaudRate = 115200;
  init.DataWidth = LL_USART_DATAWIDTH_8B;
  init.StopBits = LL_USART_STOPBITS_1;
  init.Parity = LL_USART_PARITY_NONE;
  init.TransferDirection = LL_USART_DIRECTION_TX_RX;
  init.OverSampling = LL_USART_OVERSAMPLING_16;

  init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;

  LL_USART_Disable(uart.usart);
  LL_USART_Init(uart.usart, &init);
  LL_USART_ConfigAsyncMode(uart.usart);
  LL_USART_Enable(uart.usart);

  uart.initialized = 1;
  return 1;
}


uint8_t Test_SerialWrite(QuadFC_Serial_t *serial_data)
{
  if(!uart.initialized)
  {
    return 0;
  }
  int32_t txDataLength = serial_data->bufferLength;
  uint8_t* buffer = serial_data->buffer;
  while(txDataLength > 0) 
  {
    txDataLength--;
    while (LL_USART_IsActiveFlag_TXE(uart.usart) == 0)
    {
    }

    uart.usart->DR = (*buffer++ & (uint8_t)0xFF);
    while (LL_USART_IsActiveFlag_TXE(uart.usart) == 0)
    {
    }
  }
  if (LL_USART_IsActiveFlag_TC(uart.usart) == 0)
  {
  }
  return 1;
}

void TitanTestUART_GPIOInit(USART_TypeDef* usart)
{
if(usart==UART9)
  {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_UART9);
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
    /**UART9 GPIO Configuration  
    PD14   ------> UART9_RX
    PD15   ------> UART9_TX 
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_14|LL_GPIO_PIN_15;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_11;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* UART9 interrupt Init */
    NVIC_SetPriority(UART9_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(UART9_IRQn);
  }
}
