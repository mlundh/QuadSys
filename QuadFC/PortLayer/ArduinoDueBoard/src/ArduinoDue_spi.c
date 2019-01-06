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
#include "spi_master.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <dmac.h>

static  Spi*           Spi_instances[] = {SPI0};
static  uint32_t       Spi_init[]      = {0};
static  SemaphoreHandle_t Spi_accessMutex[] = {NULL};

struct SpiMaster_SlaveDevice
{
	uint32_t baud;
	SpiMaster_BitsPerTransfer_t bitsPerTransfer;
	Spi_Mode_t mode;
	struct spi_device device;
	uint8_t selected;
};

/** DMAC receive channel of master. */
#define BOARD_SPI_DMAC_RX_CH    0
/** DMAC transmit channel of master. */
#define BOARD_SPI_DMAC_TX_CH    1

/** DMAC Channel HW Interface Number for SPI. */
#define SPI_TX_IDX              1
#define SPI_RX_IDX              2

/**
 * \brief Interrupt handler for the DMAC.
 */
void DMAC_Handler(void)
{
	portBASE_TYPE higher_priority_task_woken = pdFALSE;

	static uint32_t ul_status;

	ul_status = dmac_get_status(DMAC);
	if (ul_status & (1 << BOARD_SPI_DMAC_RX_CH)) // TODO error handling...
	{
		xSemaphoreGiveFromISR(Spi_accessMutex[0],&higher_priority_task_woken);
	}

	portEND_SWITCHING_ISR(higher_priority_task_woken);

}


static void configure_dmac(void)
{
	uint32_t ul_cfg;

	/* Initialize and enable DMA controller. */
	pmc_enable_periph_clk(ID_DMAC);
	dmac_init(DMAC);
	dmac_set_priority_mode(DMAC, DMAC_PRIORITY_ROUND_ROBIN);
	dmac_enable(DMAC);

	/* Configure DMA RX channel. */
	ul_cfg = 0;
	ul_cfg |= DMAC_CFG_SRC_PER(SPI_RX_IDX) |
			DMAC_CFG_SRC_H2SEL |
			DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
	dmac_channel_set_configuration(DMAC, BOARD_SPI_DMAC_RX_CH, ul_cfg);

	/* Configure DMA TX channel. */
	ul_cfg = 0;
	ul_cfg |= DMAC_CFG_DST_PER(SPI_TX_IDX) |
			DMAC_CFG_DST_H2SEL |
			DMAC_CFG_SOD | DMAC_CFG_FIFOCFG_ALAP_CFG;
	dmac_channel_set_configuration(DMAC, BOARD_SPI_DMAC_TX_CH, ul_cfg);

	/* Enable receive channel interrupt for DMAC. */
	NVIC_ClearPendingIRQ(DMAC_IRQn);
	NVIC_SetPriority(DMAC_IRQn, 11);
	NVIC_EnableIRQ(DMAC_IRQn);

	dmac_enable_interrupt(DMAC, (1 << BOARD_SPI_DMAC_RX_CH));
}

static void spi_master_transfer(int SpiIndex, uint8_t *p_buf, uint32_t ul_size)
{
	xSemaphoreTake(Spi_accessMutex[SpiIndex],portMAX_DELAY); // TODO timeout...
	dma_transfer_descriptor_t dmac_trans;

	dmac_channel_disable(DMAC, BOARD_SPI_DMAC_TX_CH);
	dmac_trans.ul_source_addr = (uint32_t) p_buf;
	dmac_trans.ul_destination_addr = (uint32_t) & Spi_instances[SpiIndex]->SPI_TDR;
	dmac_trans.ul_ctrlA = ul_size | DMAC_CTRLA_SRC_WIDTH_BYTE |
			DMAC_CTRLA_DST_WIDTH_BYTE;
	dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR |
			DMAC_CTRLB_FC_MEM2PER_DMA_FC |
			DMAC_CTRLB_SRC_INCR_INCREMENTING |
			DMAC_CTRLB_DST_INCR_FIXED;
	dmac_trans.ul_descriptor_addr = 0;
	dmac_channel_single_buf_transfer_init(DMAC, BOARD_SPI_DMAC_TX_CH,
			(dma_transfer_descriptor_t *) & dmac_trans);

	dmac_channel_disable(DMAC, BOARD_SPI_DMAC_RX_CH);
	dmac_trans.ul_source_addr = (uint32_t) & Spi_instances[SpiIndex]->SPI_RDR;
	dmac_trans.ul_destination_addr = (uint32_t) p_buf;
	dmac_trans.ul_ctrlA = ul_size | DMAC_CTRLA_SRC_WIDTH_BYTE |
			DMAC_CTRLA_DST_WIDTH_BYTE;
	dmac_trans.ul_ctrlB = DMAC_CTRLB_SRC_DSCR | DMAC_CTRLB_DST_DSCR |
			DMAC_CTRLB_FC_PER2MEM_DMA_FC |
			DMAC_CTRLB_SRC_INCR_FIXED |
			DMAC_CTRLB_DST_INCR_INCREMENTING;
	dmac_trans.ul_descriptor_addr = 0;
	dmac_channel_single_buf_transfer_init(DMAC, BOARD_SPI_DMAC_RX_CH,
			&dmac_trans);

	dmac_channel_enable(DMAC, BOARD_SPI_DMAC_TX_CH);
	dmac_channel_enable(DMAC, BOARD_SPI_DMAC_RX_CH);
}

uint8_t SpiMaster_Init(int SpiIndex)
{
	if(SpiIndex > 0)
	{
		return 0;
	}

	Spi_accessMutex[SpiIndex] = xSemaphoreCreateMutex();
	/* Configure DMAC. */
	configure_dmac();

	dmac_channel_disable(DMAC, BOARD_SPI_DMAC_TX_CH);
	dmac_channel_disable(DMAC, BOARD_SPI_DMAC_RX_CH);
	/* Configure an SPI peripheral. */
	Spi* p_spi = Spi_instances[SpiIndex];
	spi_disable(p_spi);
	spi_enable_clock(p_spi);
	spi_reset(p_spi);
	spi_set_master_mode(p_spi);
	spi_disable_mode_fault_detect(p_spi);
	spi_disable_loopback(p_spi);
	spi_set_peripheral_chip_select_value(p_spi, 0);
	spi_set_fixed_peripheral_select(p_spi);
	spi_disable_peripheral_select_decode(p_spi);
	spi_set_delay_between_chip_select(p_spi, CONFIG_SPI_MASTER_DELAY_BCS);
	spi_enable(p_spi);
	Spi_init[SpiIndex] = 1;
	return 1;
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
	slave->device.id = csId;
	slave->selected = 0;


	int16_t baud_div = spi_calc_baudrate_div(slave->baud, sysclk_get_cpu_hz());
	if (-1 == baud_div) {
		Assert(0 == "Failed to find baudrate divider");
	}
	spi_set_transfer_delay(Spi_instances[SpiIndex], slave->device.id, CONFIG_SPI_MASTER_DELAY_BS,
			CONFIG_SPI_MASTER_DELAY_BCT);                                                       // TODO, allow changed behavior
	spi_set_bits_per_transfer(Spi_instances[SpiIndex], slave->device.id,
			slave->bitsPerTransfer);
	spi_set_baudrate_div(Spi_instances[SpiIndex], slave->device.id, baud_div);
	spi_configure_cs_behavior(Spi_instances[SpiIndex], slave->device.id, SPI_CS_KEEP_LOW); // TODO, allow changed behavior

	uint8_t polarity;
	uint8_t phase;
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
	spi_set_clock_polarity(Spi_instances[SpiIndex], slave->device.id, polarity);
	spi_set_clock_phase(Spi_instances[SpiIndex], slave->device.id, phase);

	return slave;
}

uint8_t SpiMaster_Transfer(uint8_t SpiIndex, uint8_t *buffer, uint32_t size)
{

	if(!Spi_init[SpiIndex])
	{
		if(!SpiMaster_Init(SpiIndex))
		{
			return 0;
		}
	}

	spi_master_transfer(SpiIndex, buffer, size);
	return 1;
}

uint8_t SpiMaster_TransferPoll(uint8_t SpiIndex, uint8_t *buffer, uint32_t size)
{
	if(!Spi_init[SpiIndex])
	{
		if(!SpiMaster_Init(SpiIndex))
		{
			return 0;
		}
	}

	uint8_t uc_pcs;
	uint16_t data;

	for (uint32_t i = 0; i < size; i++)
	{
		if(spi_write(Spi_instances[SpiIndex], buffer[i], 0, 0) != SPI_OK)
		{
			return 0;
		}

		if(spi_read(Spi_instances[SpiIndex], &data, &uc_pcs) != SPI_OK )
		{
			return 0;
		}
		buffer[i] = data;
	}
	return 1;
}



void SpiMaster_SelectSlave(uint8_t SpiIndex, SpiMaster_SlaveDevice_t* slave)
{
	if(!Spi_init[SpiIndex])
	{
		if(!SpiMaster_Init(SpiIndex))
		{
			return;
		}
	}
	if(slave->selected)
	{
		return;
	}
	spi_select_device(Spi_instances[SpiIndex], &slave->device);
	slave->selected = 1;
}

void SpiMaster_DeselectSlave(uint8_t SpiIndex, SpiMaster_SlaveDevice_t* slave)
{
	if(!Spi_init[SpiIndex])
	{
		if(!SpiMaster_Init(SpiIndex))
		{
			return;
		}
	}
	if(!slave->selected)
	{
		return;
	}
	spi_deselect_device(Spi_instances[SpiIndex], &slave->device);
	slave->selected = 0;
}

