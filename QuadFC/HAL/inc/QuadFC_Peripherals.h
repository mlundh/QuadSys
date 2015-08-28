/*
 * QuadFC_Peripherals.h
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
typedef struct peripheral_i2c {
  uint8_t   internalAddr[3];      //!< Internal i2c address of the other chip.
  uint32_t  internalAddrLength;   //!< Length of the i2c internal address. 0-3 bytes supported.
  uint8_t*  buffer;               //!< Buffer containing the data to transfer.
  uint32_t  bufferLength;         //!< Number of bytes to transfer.
  uint8_t   slaveAddress;         //!< 7 bit chip address of slave node.
}QuadFC_I2C;




typedef struct peripheral_serial
{
  uint8_t*  buffer;               //!< Buffer containing the data to transfer.
  uint32_t  bufferLength;         //!< Number of bytes to transfer.
}QuadFC_Serial;

typedef struct peripheral_spi
{

}QuadFC_SPI;

uint8_t QuadFC_i2cWrite(QuadFC_I2C *i2c_data, uint8_t busIndex, TickType_t blockTimeMs);
uint8_t QuadFC_i2cRead(QuadFC_I2C *i2c_data, uint8_t busIndex, TickType_t blockTimeMs);
/*
uint8_t QuadFC_SerialWrite(QuadFC_Serial *serial_data, uint8_t busIndex, TickType_t blockTimeMs){return 0;}
uint8_t QuadFC_SerialRead(QuadFC_Serial *serial_data, uint8_t busIndex, TickType_t blockTimeMs){return 0;}
*/


#endif /* PORTLAYER_BOARD_INC_QUADFC_PERIPHERALS_H_ */
