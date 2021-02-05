/*
 * ArduinoDueSpi.c
 *
 * Copyright (C) 2017 Martin Lundh
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
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_ll_spi.h"

#include "HAL/QuadFC/QuadFC_SPI.h"
#include "FreeRTOS.h"
#include "semphr.h"

struct SpiMaster_SlaveDevice
{
	uint32_t baud;
	SpiMaster_BitsPerTransfer_t bitsPerTransfer;
	Spi_Mode_t mode;
	uint8_t selected;
};

typedef struct titan_spi_control
{
	SPI_TypeDef*				spi;
	uint32_t 					initialized;
	SemaphoreHandle_t 			transCompSem;
	int 						IRQn;
	SpiMaster_SlaveDevice_t*	CurrentSlave;
	uint8_t*					txBuff;
	uint8_t*					rxBuff;
	uint32_t					BuffLength;
	uint32_t					txIndex;
	uint32_t					rxIndex;
	uint32_t					txDone;
	uint32_t					rxDone;

} titan_spi_control_t;

const static uint8_t num_spi = 3;

static titan_spi_control_t titanSpi[] = {
	{SPI1, 0, NULL, SPI1_IRQn, 0, NULL, NULL, 0, 0, 0},
	{SPI2, 0, NULL, SPI2_IRQn, 0, NULL, NULL, 0, 0, 0},
	{SPI4, 0, NULL, SPI4_IRQn, 0, NULL, NULL, 0, 0, 0}};

void TitanSPI_GPIOInit(int SpiIndex);

void Titan_SpiISR(titan_spi_control_t *tspi);
uint32_t Titan_SPIRxCb(titan_spi_control_t *tspi);
uint32_t Titan_SPITxCb(titan_spi_control_t *tspi);

uint8_t SpiMaster_Init(int SpiIndex)
{
	if (SpiIndex > (num_spi) || titanSpi[SpiIndex].initialized)
	{
		return 0;
	}
	TitanSPI_GPIOInit(SpiIndex);
	LL_SPI_InitTypeDef SPI_InitStruct = {0};

	/* SPI1 interrupt Init */
	NVIC_SetPriority(titanSpi[SpiIndex].IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 7, 0));
	NVIC_EnableIRQ(titanSpi[SpiIndex].IRQn);

	SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
	SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
	SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
	SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
	SPI_InitStruct.NSS = LL_SPI_NSS_HARD_OUTPUT;
	SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV128;
	SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
	SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	SPI_InitStruct.CRCPoly = 10;

	titanSpi[SpiIndex].transCompSem = xSemaphoreCreateBinary();
	titanSpi[SpiIndex].initialized = 1;

	LL_SPI_Disable(titanSpi[SpiIndex].spi);
	LL_SPI_Init(titanSpi[SpiIndex].spi, &SPI_InitStruct);
	LL_SPI_SetStandard(titanSpi[SpiIndex].spi, LL_SPI_PROTOCOL_MOTOROLA);


	LL_SPI_Enable(titanSpi[SpiIndex].spi);

	return 1;
}

void TitanSPI_GPIOInit(int SpiIndex)
{
	if (titanSpi[SpiIndex].spi == SPI1)
	{
		LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

		/* Peripheral clock enable */
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);

		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
		/**SPI1 GPIO Configuration
		PA4   ------> SPI1_NSS
		PA5   ------> SPI1_SCK
		PA6   ------> SPI1_MISO
		PA7   ------> SPI1_MOSI
		*/
		GPIO_InitStruct.Pin = LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
		GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
		GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
		GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
		LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
	else if (titanSpi[SpiIndex].spi == SPI2)
	{
		LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

		/* Peripheral clock enable */
		LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
		/**SPI2 GPIO Configuration
		  PC2   ------> SPI2_MISO
		  PC3   ------> SPI2_MOSI
		  PB12   ------> SPI2_NSS
		  PB13   ------> SPI2_SCK
		  */
		GPIO_InitStruct.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3;
		GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
		GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
		GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
		LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = LL_GPIO_PIN_12 | LL_GPIO_PIN_13;
		GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
		GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
		GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
		LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
	else if (titanSpi[SpiIndex].spi == SPI4)
	{
		LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
		/* Peripheral clock enable */
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI4);
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
		/**SPI4 GPIO Configuration
		PE5   ------> SPI4_MISO
		PE6   ------> SPI4_MOSI
		PE11   ------> SPI4_NSS
		PE12   ------> SPI4_SCK
		*/
		GPIO_InitStruct.Pin = LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12;
		GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
		GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
		GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
		LL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	}
}
SpiMaster_SlaveDevice_t *SpiMaster_CreateSlaveDevice(uint8_t SpiIndex,
													 uint32_t baudRate, uint32_t csId, SpiMaster_BitsPerTransfer_t bits, Spi_Mode_t mode)
{
	if (SpiIndex > num_spi || !titanSpi[SpiIndex].initialized)
	{
		return NULL;
	}

	SpiMaster_SlaveDevice_t *slave = pvPortMalloc(sizeof(SpiMaster_SlaveDevice_t));
	if (!slave)
	{
		return NULL;
	}

	slave->baud = baudRate;
	slave->bitsPerTransfer = bits;
	slave->mode = mode;
	slave->selected = 0;

	return slave;
}

uint8_t SpiMaster_Transfer(uint8_t SpiIndex, uint8_t *rxBuffer, uint8_t *txBuffer, uint32_t size, TickType_t blockTimeMs)
{

	if (SpiIndex > num_spi || !titanSpi[SpiIndex].initialized)
	{
		return 0;
	}
	// Make sure that the semaphore is in an expected state.
	xSemaphoreTake(titanSpi[SpiIndex].transCompSem, (TickType_t)0);

	titanSpi[SpiIndex].rxBuff = rxBuffer;
	titanSpi[SpiIndex].txBuff = txBuffer;
	titanSpi[SpiIndex].BuffLength = size;
	titanSpi[SpiIndex].rxIndex = 0;
	titanSpi[SpiIndex].txIndex = 0;
	titanSpi[SpiIndex].rxDone = 0;
	titanSpi[SpiIndex].txDone = 0;
	if(rxBuffer)
	{
		// enable all isr flags at the same time.
		SET_BIT(titanSpi[SpiIndex].spi->CR2, SPI_CR2_RXNEIE | SPI_CR2_TXEIE | SPI_CR2_ERRIE);
	}
	else
	{
		SET_BIT(titanSpi[SpiIndex].spi->CR2, SPI_CR2_TXEIE | SPI_CR2_ERRIE);
	}

	// wait for transfer conplete.
	if (xSemaphoreTake(titanSpi[SpiIndex].transCompSem, pdMS_TO_TICKS(blockTimeMs)) == pdTRUE)
	{
		if(titanSpi[SpiIndex].rxBuff)
		{
			if(titanSpi[SpiIndex].rxIndex != titanSpi[SpiIndex].BuffLength)
			{
				return 0;
			}
		}
		if(titanSpi[SpiIndex].txBuff)
		{
			if(titanSpi[SpiIndex].txIndex != titanSpi[SpiIndex].BuffLength)
			{
				return 0;
			}
		}
		return 1;
	}
	else
	{
		return 0;
	}
	return 0;
}

void SpiMaster_SelectSlave(uint8_t SpiIndex, SpiMaster_SlaveDevice_t *slave)
{
	
	if(!slave)
	{
		return;
	}
	if(titanSpi[SpiIndex].CurrentSlave == slave)
	{
		LL_SPI_Enable(titanSpi[SpiIndex].spi);
		return;
	}
	LL_SPI_Disable(titanSpi[SpiIndex].spi);

	LL_SPI_InitTypeDef SPI_InitStruct = {0};

	LL_SPI_StructInit(&SPI_InitStruct);

	SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
	SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
	SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	SPI_InitStruct.NSS = LL_SPI_NSS_HARD_OUTPUT;
	SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;
	SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
	SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	SPI_InitStruct.CRCPoly = 10;


	switch (slave->mode)
	{
	case Spi_mode_0:
		SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
		SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
		break;
	case Spi_mode_1:
		SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
		SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
		break;
	case Spi_mode_2:
		SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
		SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
		break;
	case Spi_mode_3:
		SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
		SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
		break;
	}

	LL_SPI_Init(titanSpi[SpiIndex].spi, &SPI_InitStruct);
	LL_SPI_SetStandard(titanSpi[SpiIndex].spi, LL_SPI_PROTOCOL_MOTOROLA);

	LL_SPI_Enable(titanSpi[SpiIndex].spi);
	titanSpi[SpiIndex].CurrentSlave = slave;

}

void SpiMaster_DeselectSlave(uint8_t SpiIndex, SpiMaster_SlaveDevice_t *slave)
{
	LL_SPI_Disable(titanSpi[SpiIndex].spi);
}

void SPI1_IRQHandler(void)
{
	Titan_SpiISR(&titanSpi[0]);
}

void Titan_SpiISR(titan_spi_control_t *tspi)
{
	uint32_t itsource = tspi->spi->CR2;
	uint32_t itflag = tspi->spi->SR;
	portBASE_TYPE higherPriorityTaskHasWoken = pdFALSE;

	/* SPI in mode Receiver ----------------------------------------------------*/
	if ((SPI_CHECK_FLAG(itflag, SPI_FLAG_OVR) == RESET) &&
		(SPI_CHECK_FLAG(itflag, SPI_FLAG_RXNE) != RESET) &&
		(SPI_CHECK_IT_SOURCE(itsource, SPI_IT_RXNE) != RESET))
	{
		tspi->rxDone = Titan_SPIRxCb(tspi);
	}
	/* SPI in mode Transmitter -------------------------------------------------*/
	if ((SPI_CHECK_FLAG(itflag, SPI_FLAG_TXE) != RESET) &&
		(SPI_CHECK_IT_SOURCE(itsource, SPI_IT_TXE) != RESET))
	{
		tspi->txDone = Titan_SPITxCb(tspi);
	}



	/* SPI in Error Treatment --------------------------------------------------*/
	if (((SPI_CHECK_FLAG(itflag, SPI_FLAG_MODF) != RESET) ||
		 (SPI_CHECK_FLAG(itflag, SPI_FLAG_OVR) != RESET) ||
		 (SPI_CHECK_FLAG(itflag, SPI_FLAG_FRE) != RESET)) &&
		(SPI_CHECK_IT_SOURCE(itsource, SPI_IT_ERR) != RESET))
	{
		/* SPI Overrun error interrupt occurred ----------------------------------*/
		if (SPI_CHECK_FLAG(itflag, SPI_FLAG_OVR) != RESET)
		{
			LL_SPI_ClearFlag_OVR(tspi->spi);
		}
		/* SPI Mode Fault error interrupt occurred -------------------------------*/
		if (SPI_CHECK_FLAG(itflag, SPI_FLAG_MODF) != RESET)
		{
			LL_SPI_ClearFlag_MODF(tspi->spi);
		}
		/* SPI Frame error interrupt occurred ------------------------------------*/
		if (SPI_CHECK_FLAG(itflag, SPI_FLAG_FRE) != RESET)
		{
			LL_SPI_ClearFlag_FRE(tspi->spi);
		}
	}

	// If the transmission is complete then notify the initiator.
	if((tspi->rxBuff && tspi->txBuff  && tspi->rxDone && tspi->txDone)	// RX TX mode done
		|| (tspi->rxBuff && !tspi->txBuff && tspi->rxDone)		// RX only done
		|| (!tspi->rxBuff && tspi->txBuff && tspi->txDone) )		// TX only done
	{

		while ((tspi->spi->SR & SPI_SR_TXE) == RESET)
		{};
		while(LL_SPI_IsActiveFlag_BSY(tspi->spi))
		{};
		xSemaphoreGiveFromISR(tspi->transCompSem, &higherPriorityTaskHasWoken);
		

	}
	/* If giving the semaphore triggered a higher-priority task to be unblocked
    then this call will ensure that the isr returns to the task with higer priority.*/		
	if(higherPriorityTaskHasWoken == pdTRUE)
	{
		portEND_SWITCHING_ISR(higherPriorityTaskHasWoken);
	}
}

uint32_t Titan_SPIRxCb(titan_spi_control_t *tspi)
{
	if(tspi->rxBuff)
	{
		tspi->rxBuff[tspi->rxIndex++] = LL_SPI_ReceiveData8(tspi->spi);	
	}
	else
	{
		tspi->rxIndex++;
		LL_SPI_ReceiveData8(tspi->spi);	// Ignore read if we do not have a buffer.
	}
	if(tspi->rxIndex >= tspi->BuffLength)
	{
		CLEAR_BIT(tspi->spi->CR2, SPI_CR2_RXNEIE | SPI_CR2_ERRIE);
		return 1;
	}
	return 0;
}

uint32_t Titan_SPITxCb(titan_spi_control_t *tspi)
{
	if(tspi->txBuff)
	{
		LL_SPI_TransmitData8(tspi->spi, tspi->txBuff[tspi->txIndex++]);
	}
	else
	{
		tspi->txIndex++;
		LL_SPI_TransmitData8(tspi->spi, 0);
	}
	if(tspi->txIndex >= tspi->BuffLength)
	{
		LL_SPI_DisableIT_TXE(tspi->spi);
		if(!tspi->rxBuff)
		{
			LL_SPI_ClearFlag_OVR(tspi->spi);
		}

		return 1;
	}
	return 0;
}
