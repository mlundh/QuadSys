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
#include "stm32f413xx.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"
#include "Components/Utilities/inc/Utilities_CharCircularBuffer.h"


typedef struct titan_uart_control {
  USART_TypeDef*        usart;
  uint32_t              initialized;
  SemaphoreHandle_t     rx_trans_comp_sem;
  SemaphoreHandle_t     tx_trans_comp_sem;
  uint8_t*              TxBuffPtr;
  uint32_t              TxCount;
  CharCircBuffer_t*     RxBuff;
} titan_uart_control_t;

const static uint8_t num_uart = 7;

static  titan_uart_control_t   uart[] = {
  {UART10, 0, NULL, NULL, NULL, 0, NULL}, // RC 1
  {USART1, 0, NULL, NULL, NULL, 0, NULL}, // Com serial USB
  {USART2, 0, NULL, NULL, NULL, 0, NULL},
  {UART9,  0, NULL, NULL, NULL, 0, NULL}, // Applog serial backend
  {USART3, 0, NULL, NULL, NULL, 0, NULL},
  {USART6, 0, NULL, NULL, NULL, 0, NULL},
  {UART8,  0, NULL, NULL, NULL, 0, NULL},

};

#define RECEIVE_BUFFER_LENGTH (512)

/**
 * @brief Local uart handler, called by all uart peripherals.
 * 
 * @param usart_index 
 */

static uint32_t Parity[] =
{
    LL_USART_PARITY_NONE,
    LL_USART_PARITY_EVEN,
    LL_USART_PARITY_ODD
};

static uint32_t StopBits[] =
{
    LL_USART_STOPBITS_0_5,
    LL_USART_STOPBITS_1,
    LL_USART_STOPBITS_2
};

static uint32_t WordLength[] =
{
    0,
    0,
    0,
    LL_USART_DATAWIDTH_8B,
    LL_USART_DATAWIDTH_9B
};

void TitanSerial_IRQHandler(uint32_t busIndex);

void TitanUART_GPIOInit(USART_TypeDef* usart);


uint8_t QuadFC_SerialInit(int busIndex, QuadFC_SerialOptions_t* opt)
{
  if(busIndex > (num_uart) || uart[busIndex].initialized)
  {
    return 0;
  }
  uart[busIndex].RxBuff = Utilities_CBuffCreate(opt->bufferLength);
  
  TitanUART_GPIOInit(uart[busIndex].usart);

  LL_USART_InitTypeDef  init = {0};

  init.BaudRate = opt->baudRate;
  init.DataWidth = WordLength[opt->dataBits];
  init.StopBits = StopBits[opt->stopBits];
  init.Parity = Parity[opt->parityType];
  init.TransferDirection = LL_USART_DIRECTION_TX_RX;
  init.OverSampling = LL_USART_OVERSAMPLING_16;
  uart[busIndex].rx_trans_comp_sem = xSemaphoreCreateCounting(portMAX_DELAY, 0);
  uart[busIndex].tx_trans_comp_sem = xSemaphoreCreateCounting(portMAX_DELAY, 0);

  if((uart[busIndex].usart == USART1) ||
     (uart[busIndex].usart == USART2))
  {
    init.HardwareFlowControl = LL_USART_HWCONTROL_RTS_CTS;
  }
 // else
  {
    init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  }
  LL_USART_Disable(uart[busIndex].usart);
  LL_USART_Init(uart[busIndex].usart, &init);
  LL_USART_ConfigAsyncMode(uart[busIndex].usart);
  LL_USART_Enable(uart[busIndex].usart);

  // RX is enabled from the start, and will always read to the circular buffer.
  
  LL_USART_EnableIT_PE(uart[busIndex].usart); 
  LL_USART_EnableIT_ERROR(uart[busIndex].usart); 
  LL_USART_EnableIT_RXNE(uart[busIndex].usart); 

  uart[busIndex].initialized = 1;
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

  uart[busIndex].TxBuffPtr = serial_data->buffer;
  uart[busIndex].TxCount = serial_data->bufferLength;
  
  LL_USART_EnableIT_TXE(uart[busIndex].usart); // This causes an interrupt that starts the transmission.
  
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

  // TODO copy all avalible data.
  if( xSemaphoreTake( uart[busIndex].tx_trans_comp_sem, ( TickType_t ) blockTimeTicks ) == pdTRUE )
  {
    Utilities_CBuffPop(uart[busIndex].RxBuff, serial_data->buffer, serial_data->bufferLength) ;
  }
  else
  {
    return 0;
  }
  return 0;
}

void USART1_IRQHandler(void)
{
  TitanSerial_IRQHandler(1);
}

void USART2_IRQHandler(void)
{
  TitanSerial_IRQHandler(2);
}

void USART3_IRQHandler(void)
{
  TitanSerial_IRQHandler(4);
}

void USART6_IRQHandler(void)
{
  TitanSerial_IRQHandler(5);
}

void UART8_IRQHandler(void)
{
  TitanSerial_IRQHandler(6);
}

void UART9_IRQHandler(void)
{
  TitanSerial_IRQHandler(3);
}

void UART10_IRQHandler(void)
{
  TitanSerial_IRQHandler(0);
}

void TitanSerial_IRQHandler(uint32_t busIndex)
{ 
  portBASE_TYPE higherPriorityTaskHasWoken = pdFALSE;
  uint32_t isrflags   = READ_REG(uart[busIndex].usart->SR);
  uint32_t errorflags = 0x00U;

  /* If no error occurs */
  errorflags = (isrflags & (uint32_t)(USART_SR_PE | USART_SR_FE | USART_SR_ORE | USART_SR_NE));
  if (errorflags == RESET)
  {
    /* UART in mode Receiver -------------------------------------------------*/
    if (LL_USART_IsActiveFlag_RXNE(uart[busIndex].usart) && LL_USART_IsEnabledIT_RXNE(uart[busIndex].usart) )
    {
      uint8_t result = 1;
      if (LL_USART_GetParity(uart[busIndex].usart) == LL_USART_PARITY_NONE)
      {
        result = Utilities_CBuffPush(uart[busIndex].RxBuff, (uint8_t)(uart[busIndex].usart->DR & (uint8_t)0x00FF));
      }
      else
      {
        result = Utilities_CBuffPush(uart[busIndex].RxBuff, (uint8_t)(uart[busIndex].usart->DR & (uint8_t)0x007F));
      }
      if(result) // Only signal new data if we have increased the count. 
      {
        xSemaphoreGiveFromISR(uart[busIndex].rx_trans_comp_sem, &higherPriorityTaskHasWoken);
      }
    }
  }
  else
  {
    // Just clear errors for now.
    if(LL_USART_IsActiveFlag_FE(uart[busIndex].usart) && LL_USART_IsEnabledIT_RXNE(uart[busIndex].usart))
    {
      LL_USART_ClearFlag_FE(uart[busIndex].usart);
    }
    if(LL_USART_IsActiveFlag_PE(uart[busIndex].usart)&& LL_USART_IsEnabledIT_PE(uart[busIndex].usart))
    {
      LL_USART_ClearFlag_PE(uart[busIndex].usart);
    }
    if(LL_USART_IsActiveFlag_NE(uart[busIndex].usart)&& LL_USART_IsEnabledIT_RXNE(uart[busIndex].usart))
    {
      LL_USART_ClearFlag_NE(uart[busIndex].usart);
    }
    if(LL_USART_IsActiveFlag_ORE(uart[busIndex].usart)&& LL_USART_IsEnabledIT_RXNE(uart[busIndex].usart))
    {
      LL_USART_ClearFlag_ORE(uart[busIndex].usart);
    }
  }
  /* UART in mode Transmitter ------------------------------------------------*/
  if (LL_USART_IsActiveFlag_TXE(uart[busIndex].usart) && LL_USART_IsEnabledIT_TXE(uart[busIndex].usart) )
  {
    LL_USART_TransmitData8(uart[busIndex].usart, (uint8_t)(*uart[busIndex].TxBuffPtr++ & (uint8_t)0x00FF));
    if (--uart[busIndex].TxCount == 0U)
    {
      /* Disable the UART TX Empty Interrupt */
      LL_USART_DisableIT_TXE(uart[busIndex].usart);

      /* Enable the UART Transmit Complete Interrupt */
      LL_USART_EnableIT_TC(uart[busIndex].usart);
    }
  }
  /* UART in mode Transmitter end --------------------------------------------*/
  if (LL_USART_IsActiveFlag_TC(uart[busIndex].usart) && LL_USART_IsEnabledIT_TC(uart[busIndex].usart) )
  {
      LL_USART_DisableIT_TC(uart[busIndex].usart);

      xSemaphoreGiveFromISR(uart[busIndex].tx_trans_comp_sem, &higherPriorityTaskHasWoken);
      /* If giving the semaphore triggered a higher-priority task to be unblocked
      then this call will ensure that the isr returns to the task with higer priority.*/
  
  }
  if(higherPriorityTaskHasWoken == pdTRUE)
  {
    portEND_SWITCHING_ISR(higherPriorityTaskHasWoken);
  }
}




void TitanUART_GPIOInit(USART_TypeDef* usart)
{
  if(usart==UART8)
  {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART8);
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
    /**UART8 GPIO Configuration  
    PE0   ------> UART8_RX
    PE1   ------> UART8_TX 
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* UART8 interrupt Init */
    NVIC_SetPriority(UART8_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(UART8_IRQn);
  }
  else if(usart==UART9)
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
  else if(usart==UART10)
  {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_UART10);
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
    /**UART10 GPIO Configuration  
    PE2   ------> UART10_RX
    PE3   ------> UART10_TX 
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_3;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_11;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* UART10 interrupt Init */
    NVIC_SetPriority(UART10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(UART10_IRQn);
  }
  else if(usart==USART1)
  {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    /**USART1 GPIO Configuration  
    PA10   ------> USART1_RX
    PA11   ------> USART1_CTS
    PA12   ------> USART1_RTS
    PA15   ------> USART1_TX 
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_10|LL_GPIO_PIN_15;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_11|LL_GPIO_PIN_12;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(USART1_IRQn);
  }
  else if(usart==USART2)
  {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
    /**USART2 GPIO Configuration  
    PD5   ------> USART2_TX
    PD6   ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5|LL_GPIO_PIN_6;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(USART2_IRQn);
  }
  else if(usart==USART3)
  {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
    /**USART3 GPIO Configuration  
    PC5   ------> USART3_RX
    PB14   ------> USART3_RTS
    PD8   ------> USART3_TX
    PD11   ------> USART3_CTS 
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(USART3_IRQn);
  }
  else if(usart==USART6)
  {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    /**USART6 GPIO Configuration  
    PC6   ------> USART6_TX
    PC7   ------> USART6_RX 
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
    LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART6 interrupt Init */
    NVIC_SetPriority(USART6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
    NVIC_EnableIRQ(USART6_IRQn);
  }
}
