/*
* QuadFC_I2c.h
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
#ifndef __QUADFC_I2C_H__
#define __QUADFC_I2C_H__

#include "stddef.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "HAL/QuadFC/QuadFC_PeripheralsCommon.h"


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

typedef enum{
  scl,    //!< Data pin.
  sda,    //!< Clock pin.
} QuadFC_i2cPins_t;




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
 * @brief Reconfigure the pins used by busIndex to take sw control of the output state of them. Used
 * for special cases, for example when we need to clock the bus to un-stuck a slave.
 * 
 * @param busIndex i2c bus index on the board.
 * @param config   configuration the pins should be set to.
 * @return int     0 if fail, 1 otherwise.
 */
int QuadFC_i2cReconfigPin(int busIndex, QuadFC_PinConfig_t config);

/**
 * @brief Set the pin to the desired state.
 * 
 * @param busIndex  i2c bus index on the board.
 * @param state     1 if high, 0 if low.
 * @param pin       Pin to set.
 * @return int 
 */
int QuadFC_i2cSetPin(int busIndex, QuadFC_PinState_t state, QuadFC_i2cPins_t pin);

#endif // __QUADFC_I2C_H__
