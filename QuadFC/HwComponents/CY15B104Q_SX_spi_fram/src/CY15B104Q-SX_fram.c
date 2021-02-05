/*
 * MB85RS64V_fram.c
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
#include "CY15B104Q_SX_spi_fram/inc/CY15B104Q-SX_fram.h"
#include "HAL/QuadFC/QuadFC_SPI.h"
static SpiMaster_SlaveDevice_t *slave = NULL;
#define SPI_INDEX_FRAM 0
#define BAUD_RATE 10000000
#define BITS_PER_TRANSFER 8
#define CHIP_SELECT_ID 0

uint8_t CY15B104Q_SX_init()
{
  slave = SpiMaster_CreateSlaveDevice(SPI_INDEX_FRAM, BAUD_RATE, CHIP_SELECT_ID, BITS_PER_TRANSFER, Spi_mode_0);
  if (!slave)
  {
    return 0;
  }
  SpiMaster_DeselectSlave(SPI_INDEX_FRAM, slave);
  return 1;
}

uint8_t CY15B104Q_SX_MemRead(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size)
{
  if (!slave)
  {
    return 0;
  }
  SpiMaster_SelectSlave(SPI_INDEX_FRAM, slave);
  uint8_t startWriteBuff[4] = {0};
  startWriteBuff[0] = OPCODE_READ;
  startWriteBuff[1] = (uint8_t)(addr >> 16);
  startWriteBuff[2] = (uint8_t)(addr >> 8);
  startWriteBuff[3] = (uint8_t)(addr & 0xFF);
  uint8_t result = SpiMaster_Transfer(SPI_INDEX_FRAM, NULL, startWriteBuff, 4, 1);
  result &= SpiMaster_Transfer(SPI_INDEX_FRAM, buffer, NULL, size, 1);
  SpiMaster_DeselectSlave(SPI_INDEX_FRAM, slave);

  return result;
}

uint8_t CY15B104Q_SX_MemWrite(uint32_t addr, uint32_t size, uint8_t *buffer, uint32_t buffer_size)
{
  if (!slave)
  {
    return 0;
  }
  CY15B104Q_SX_WriteEnable(1); // write is disabled after each write operaton. It has to be enabled for each new write.
  SpiMaster_SelectSlave(SPI_INDEX_FRAM, slave);
  uint8_t startWriteBuff[4] = {0};
  startWriteBuff[0] = OPCODE_WRITE;
  startWriteBuff[1] = (uint8_t)(addr >> 16);
  startWriteBuff[2] = (uint8_t)(addr >> 8);
  startWriteBuff[3] = (uint8_t)(addr & 0xFF);
  uint8_t result = SpiMaster_Transfer(SPI_INDEX_FRAM, NULL, startWriteBuff, 4, 1);
  result &= SpiMaster_Transfer(SPI_INDEX_FRAM, NULL, buffer, size, 1);
  SpiMaster_DeselectSlave(SPI_INDEX_FRAM, slave);

  return result;
}

uint8_t CY15B104Q_SX_MemReadWriteRaw(uint8_t *RxBuffer, uint8_t *TxBuffer, uint32_t buffer_size)
{
  SpiMaster_SelectSlave(SPI_INDEX_FRAM, slave);
  uint8_t result = SpiMaster_Transfer(SPI_INDEX_FRAM, RxBuffer, TxBuffer, buffer_size, 100);
  SpiMaster_DeselectSlave(SPI_INDEX_FRAM, slave);
  return result;
}



uint8_t CY15B104Q_SX_WriteEnable(uint8_t enable)
{
  if (!slave)
  {
    return 0;
  }
  SpiMaster_SelectSlave(SPI_INDEX_FRAM, slave);

  uint8_t data = enable ? OPCODE_WREN : OPCODE_WRDI;
  
  uint8_t result = SpiMaster_Transfer(SPI_INDEX_FRAM, NULL, &data, 1, 1);
  
  SpiMaster_DeselectSlave(SPI_INDEX_FRAM, slave);

  return result;
}

uint8_t CY15B104Q_SX_TestWriteEnable()
{
  if (!slave)
  {
    return 0;
  }
  uint8_t result = 0;
  CY15B104Q_SX_WriteEnable(1);
  SpiMaster_SelectSlave(SPI_INDEX_FRAM, slave);
  uint8_t data = OPCODE_RDSR;
  result = SpiMaster_Transfer(SPI_INDEX_FRAM, NULL, &data, 1, 1);
  result = SpiMaster_Transfer(SPI_INDEX_FRAM, &data, NULL, 1, 1);
  SpiMaster_DeselectSlave(SPI_INDEX_FRAM, slave);
  SpiMaster_SelectSlave(SPI_INDEX_FRAM, slave);
  data = OPCODE_RDSR;
  result = SpiMaster_Transfer(SPI_INDEX_FRAM, NULL, &data, 1, 1);
  result = SpiMaster_Transfer(SPI_INDEX_FRAM, &data, NULL, 1, 1);
  SpiMaster_DeselectSlave(SPI_INDEX_FRAM, slave);

  return result;
}
