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

#include "HAL/QuadFC/QuadFC_Peripherals.h"
#include "FreeRTOS.h"
#include "semphr.h"

static  uint32_t       Spi_init[]      = {0};
static  SemaphoreHandle_t Spi_accessMutex[] = {NULL};

struct SpiMaster_SlaveDevice
{
	uint32_t baud;
	SpiMaster_BitsPerTransfer_t bitsPerTransfer;
	Spi_Mode_t mode;
	uint8_t selected;
};

static void spi_master_transfer(int SpiIndex, uint8_t *p_buf, uint32_t ul_size)
{

}

uint8_t SpiMaster_Init(int SpiIndex)
{
	return 0;
}


SpiMaster_SlaveDevice_t* SpiMaster_CreateSlaveDevice(uint8_t SpiIndex,
		uint32_t baudRate, uint32_t csId, SpiMaster_BitsPerTransfer_t bits, Spi_Mode_t mode)
{
	if(!Spi_init[SpiIndex])
	{
		if(!SpiMaster_Init(SpiIndex))
		{
			return NULL;
		}
	}
	SpiMaster_SlaveDevice_t* slave = pvPortMalloc(sizeof(SpiMaster_SlaveDevice_t));
	if(!slave)
	{
		return NULL;
	}
	slave->baud = baudRate;
	slave->bitsPerTransfer = bits;
	slave->mode = mode;
	slave->selected = 0;



	uint8_t polarity = 0;
	uint8_t phase = 0;
	switch (slave->mode)
	{
	case Spi_mode_0:
		polarity = 0;
		phase = 1;
		break;
	case Spi_mode_1:
		polarity = 0;
		phase = 0;
		break;
	case Spi_mode_2:
		polarity = 1;
		phase = 1;
		break;
	case Spi_mode_3:
		polarity = 1;
		phase = 0;
		break;
	}

	return NULL;
}

uint8_t SpiMaster_Transfer(uint8_t SpiIndex, uint8_t *buffer, uint32_t size)
{
	return 0;
}

uint8_t SpiMaster_TransferPoll(uint8_t SpiIndex, uint8_t *buffer, uint32_t size)
{
	return 0;
}



void SpiMaster_SelectSlave(uint8_t SpiIndex, SpiMaster_SlaveDevice_t* slave)
{

}

void SpiMaster_DeselectSlave(uint8_t SpiIndex, SpiMaster_SlaveDevice_t* slave)
{

}

