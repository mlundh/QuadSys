/*
 * QuadFC_Peripherals.h
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
#ifndef PORTLAYER_BOARD_INC_QUADFC_PERIPHERALS_H_
#define PORTLAYER_BOARD_INC_QUADFC_PERIPHERALS_H_
#include "stddef.h"
#include "stdint.h"
#include "FreeRTOS.h"

/**
 * @struct periperal_i2c
 * Internal address is non-standard for the i2c bus, but
 * most chip support/demand a message format with it, so
 * it is supported here as well.
 */
typedef struct{
  uint8_t   internalAddr[3];      //!< Internal i2c address of the other chip.
  uint32_t  internalAddrLength;   //!< Length of the i2c internal address. 0-3 bytes supported.
  uint8_t*  buffer;               //!< Buffer containing the data to transfer.
  uint32_t  bufferLength;         //!< Number of bytes to transfer.
  uint8_t   slaveAddress;         //!< 7 bit chip address of slave node.
}QuadFC_I2C_t;

/**
 * @struct QuadFC_Serial_t
 * Container for information needed by the read/write functionality of
 * the serial port.
 */
typedef struct{
  uint8_t*  buffer;               //!< Buffer containing the data to transfer.
  uint32_t  bufferLength;         //!< Number of bytes to transfer.
}QuadFC_Serial_t;

/**
 * @enum
 * Enum used for initialization of a uart instance. Defines
 * the allowed parity values.
 */
typedef enum{
  NoParity = 0,//!< NoParity
  OddParity,   //!< OddParity
  EvenParity,  //!< EvenParity
}QuadFC_SerialParity_t;

/**
 * @enum
 * Enum used for initialization of a uart instance. Defines
 * the allowed number of data bits in a char.
 */
typedef enum{
  FiveDataBits = 0,
  SixDataBits,
  SevenDataBits,
  EightDataBits,
}QuadFC_SerialDataBits_t;

/**
 * @enum
 * Enum used for initialization of a uart instance. Defines
 * the allowed number of stop bits.
 */
typedef enum{
  OneStopBit = 0,
  OneAndHalfStopBits,
  TwoStopBits,
}QuadFC_SerialStopBits_t;

/**
 * @enum
 * Enum used for initialization of a uart instance. Defines
 * the allowed flow control.
 */
typedef enum{
  NoFlowControl = 0,
}QuadFC_SerialFlowControl_t;

/**
 * @struct QuadFC_SerialOptions_t
 * Struct containing the settings for a serial port.
 */
typedef struct peripheral_serial_options
{
  uint32_t baudRate;
  QuadFC_SerialDataBits_t dataBits;
  QuadFC_SerialParity_t parityType;
  QuadFC_SerialStopBits_t stopBits;
  QuadFC_SerialFlowControl_t flowControl;
  uint8_t *receiveBuffer;
  uint32_t bufferLength;
}QuadFC_SerialOptions_t;

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
 * Initialize the i2c bus on index busIndex.
 * @param busIndex    Index of the buss the device is connected to.
 * @param speed       Speed of the buss.
 * @return            1 if success, 0 otherwise.
 */
uint8_t QuadFC_i2cInit(int busIndex, int speed);

/**
 * Send data over the i2c bus.
 * @param i2c_data      Struct containing the data and address etc needed.
 * @param busIndex      i2c bus index on the board.
 * @param blockTimeMs   Time to block. 0 meand indefenetly.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t QuadFC_i2cWrite(QuadFC_I2C_t *i2c_data, uint8_t busIndex, TickType_t blockTimeMs);

/**
 * read data from the i2c bus.
 * @param i2c_data      Struct containing the data and address etc needed.
 * @param busIndex      i2c bus index on the board.
 * @param blockTimeMs   Time to block. 0 meand indefenetly.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t QuadFC_i2cRead(QuadFC_I2C_t *i2c_data, uint8_t busIndex, TickType_t blockTimeMs);

/**
 * Serial interface used by the test framework. Implementation should not rely on FreeRTOS.
 * @return
 */
uint8_t Test_SerialInit();

/**
 * Write data to the serial interface used by the test framework. Implementation should
 * not rely on FreeRTOS.
 * @param serial_data   Data to be transmitted.
 * @return
 */
uint8_t Test_SerialWrite(QuadFC_Serial_t *serial_data);

/**
 * Initialize the serial port on index busIndex.
 * @param busIndex    Index of the serial port the device is connected to.
 * @param opt         Serial options.
 * @return            1 if success, 0 otherwise.
 */
uint8_t QuadFC_SerialInit(int busIndex, QuadFC_SerialOptions_t* opt);

/**
 * Write data to the serial interface.
 * @param serial_data      Struct containing the data.
 * @param busIndex      Serial index on the board.
 * @param blockTimeMs   Time to block. 0 meand indefenetly.
 * @return              0 if fail, 1 otherwise.
 */
uint8_t QuadFC_SerialWrite(QuadFC_Serial_t *serial_data, uint8_t busIndex, TickType_t blockTimeMs);

/**
 * Read data from the serial interface.
 * @param serial_data      Struct containing the data.
 * @param busIndex      Serial index on the board.
 * @param blockTimeMs   Time to block. 0 meand indefenetly.
 * @return              0 if fail, 1 otherwise.
 */
uint32_t QuadFC_SerialRead(QuadFC_Serial_t *serial_data, uint8_t busIndex, TickType_t blockTimeMs);

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
 * Transfer data to the last selected slave. Bytes from buffer will be written, and at the same
 * time buffer will be populated by the incoming data from the slave. Non blocking version, uses DMA.
 * @param SpiIndex  Bus index.
 * @param buffer    Buffer to write.
 * @param size      Size of the buffer.
 * @return          1 if success, 0 otherwise.
 */
uint8_t SpiMaster_Transfer(uint8_t SpiIndex, uint8_t *buffer, uint32_t size);

/**
 * Transfer data to the last selected slave. Bytes from buffer will be written, and at the same
 * time buffer will be populated by the incoming data from the slave. Blocking verstion.
 * @param SpiIndex  Bus index.
 * @param buffer    Buffer to write.
 * @param size      Size of the buffer.
 * @return          1 if success, 0 otherwise.
 */
uint8_t SpiMaster_TransferPoll(uint8_t SpiIndex, uint8_t *buffer, uint32_t size);

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


#endif /* PORTLAYER_BOARD_INC_QUADFC_PERIPHERALS_H_ */
