/*
* BoardConfig.h
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
#ifndef __BOARDCONFIG_H__
#define __BOARDCONFIG_H__

//i2c bus defines
#define IMU1_I2C_BUS (0)
#define IMU2_I2C_BUS (0)
#define FRAM_MEM_I2C_BUS (0)

//SPI bus defines
#define FRAM_MEM_SPI_BUS (0)

//Serial bus defines
#define RC1_SERIAL_BUS (0)
#define RC2_SERIAL_BUS (2)
#define COM_SERIAL_BUS (1)
#define APPLOG_SERIAL_BUS (3)

// mem area defines
#define PARAM_MEM_START_ADDR (0)
#define LOG_MEM_START_ADDR (32000)

#endif // __BOARDCONFIG_H__
