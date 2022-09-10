/*
* QuadFC_Serial.h
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
#ifndef __QUADFC_SERIAL_H__
#define __QUADFC_SERIAL_H__

#include "stddef.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "HAL/QuadFC/QuadFC_PeripheralsCommon.h"


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
  OneHalfStopBit,
  OneStopBit,
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
  uint8_t *receiveBuffer;                   // Only used if the port needs it. 
  uint32_t bufferLength;                    //Used either to determine the length of receive buffer, or the trigger level. 
}QuadFC_SerialOptions_t;

typedef enum{
  rx,    //!< receive pin.
  tx,    //!< transmitt pin.
} QuadFC_SerialPins_t;

/**
 * @struct QuadFC_Serial_t
 * Container for information needed by the read/write functionality of
 * the serial port.
 */
typedef struct{
  uint8_t*  buffer;               //!< Buffer containing the data to transfer.
  uint32_t  bufferLength;         //!< Number of bytes to transfer.
  uint32_t  triggerLevel;
}QuadFC_Serial_t;


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
 * @brief Deinitialize a serial interface. Will not re-set the pin configuration. 
 * 
 * @param busIndex    Index of the serial port the device is connected to.
 * @return uint8_t    1 if success, 0 otherwise.
 */
uint8_t QuadFC_SerialDeInit(int busIndex);


/**
 * @brief Reconfigure the pins used by the serial bus identified by busIndex.
 * 
 * @param busIndex    Index of the serial port the device is connected to.
 * @param config      Config of the pin.
 * @return int 
 */
int QuadFC_SerialReconfigPin(int busIndex, QuadFC_PinConfig_t config);

/**
 * @brief Set the pin state of the desired pin. 
 * 
 * the serial port pins must first have been configured to output.
 * 
 * @param busIndex  Index of the serial port the device is connected to.
 * @param state     State to set on the pin.  
 * @param pin       rx or tx.
 * @return int 
 */
int QuadFC_SerialSetPin(int busIndex, QuadFC_PinState_t state, QuadFC_SerialPins_t pin);

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


#endif // __QUADFC_SERIAL_H__