/*
* QuadFC_SPI.h
*
* Copyright (C) 2021  Martin Lundh
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

#ifndef __QUADFC_SPI_H__
#define __QUADFC_SPI_H__

#include "stddef.h"
#include "stdint.h"
#include "FreeRTOS.h"

typedef struct SpiMaster_SlaveDevice SpiMaster_SlaveDevice_t;

typedef enum Spi_BitsPerTransfer
{
  Spi_8_bit_per_transfer,
  Spi_9_bit_per_transfer,
  Spi_10_bit_per_transfer,
  Spi_11_bit_per_transfer,
  Spi_12_bit_per_transfer,
  Spi_13_bit_per_transfer,
  Spi_14_bit_per_transfer,
  Spi_15_bit_per_transfer,
  Spi_16_bit_per_transfer,
}SpiMaster_BitsPerTransfer_t;


typedef enum Spi_Mode
{
  Spi_mode_0,
  Spi_mode_1,
  Spi_mode_2,
  Spi_mode_3
}Spi_Mode_t;

/**
 * Initialize the Spi master on the spi bus denoted by the index.
 * @param SpiIndex  Index of the bus.
 * @return
 */
uint8_t SpiMaster_Init(int SpiIndex);

/**
 * Create an spi slave device. Use this device to send data to the peripheral module.
 * @param SpiIndex  Bus index the peripheral is connected to.
 * @param baudRate  Baud rate to use when communicating with the peripheral.
 * @param csId      Chip select id.
 * @param bits      Bits per transfer.
 * @param mode      SPI mode.
 * @return          Pointer to the initialized slave device. Null if fail.
 */
SpiMaster_SlaveDevice_t* SpiMaster_CreateSlaveDevice(uint8_t SpiIndex,
    uint32_t baudRate, uint32_t csId, SpiMaster_BitsPerTransfer_t bits, Spi_Mode_t mode);

/**
 * Transfer data to the last selected slave. Bytes from the tx buffer will be written, and at the same
 * time the rx buffer will be populated by the incoming data from the slave. 
 * 
 * If only tx or rx should be used, then pass a NULL pointer to the other. 
 * @param SpiIndex  Bus index.
 * @param buffer    Buffer to write.
 * @param size      Size of the buffer.
 * @return          1 if success, 0 otherwise.
 */
uint8_t SpiMaster_Transfer(uint8_t SpiIndex, uint8_t *rxBuffer, uint8_t *txBuffer, uint32_t size, TickType_t blockTimeMs);

/**
 * Select the slave given by the parameters.
 * @param SpiIndex  Bus index.
 * @param slave     Slave to select.
 */
void SpiMaster_SelectSlave(uint8_t SpiIndex, SpiMaster_SlaveDevice_t* slave);

/**
 * Deselect the slave.
 * @param SpiIndex  Bus index.
 * @param slave     Slave to deselect.
 */
void SpiMaster_DeselectSlave(uint8_t SpiIndex, SpiMaster_SlaveDevice_t* slave);


#endif // __QUADFC_SPI_H__
