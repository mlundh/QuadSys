/*
 * board_test.c
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
#include "Test/ArduinoDueBoard/board_test.h"
#include "string.h"
#include "HAL/QuadFC/QuadFC_Memory.h"
void Board_GetTCs(TestFw_t* obj)
{
  TestFW_RegisterTest(obj, "SPI fram", Board_TestSpiFram_1);
  TestFW_RegisterTest(obj, "SPI fram", Board_TestSpiFram_2);

}
#define BASE_ADDR_SPI 32000

#define ADDR_1 500
#define ADDR_2 50

uint8_t Board_TestSpiFram_1(TestFw_t* obj)
{
  uint8_t result = 1;
  {
    uint8_t data [8] = {0};
    uint8_t data_verify [8] = {0};

    Mem_Write(BASE_ADDR_SPI+ADDR_1, 8, data, 8);

    uint8_t data_read [] = {0xff,0xee,0xee,0xdd,0xff,0xee,0xee,0xdd};
    Mem_Read(BASE_ADDR_SPI+ADDR_1,8, data_read, 8);

    if(memcmp(data_verify, data_read, 8) != 0)
    {
      result = 0;
    }

  }
  Mem_Init();
  {
    uint8_t data [4] = {0};
    uint8_t data_verify [4] = {0};
    Mem_Write(BASE_ADDR_SPI + ADDR_2, 4, data, 4);

    uint8_t data_read [] = {0xff,0xee,0xee,0xdd};
    Mem_Read(BASE_ADDR_SPI + ADDR_2, 4, data_read, 4);

    if(memcmp(data_verify, data_read, 4) != 0)
    {
      result = 0;
    }

  }
  return result;
}


uint8_t Board_TestSpiFram_2(TestFw_t* obj)
{
  uint8_t result = 1;
  {
    uint8_t data [] = {0xff,0xee,0xee,0xdd,0xff,0xee,0xee,0xdd};
    uint8_t data_verify [] = {0xff,0xee,0xee,0xdd,0xff,0xee,0xee,0xdd};

    Mem_Write(BASE_ADDR_SPI+ADDR_1, 8, data, 8);

    uint8_t data_read [8] = {0};
    Mem_Read(BASE_ADDR_SPI+ADDR_1,8, data_read, 8);

    if(memcmp(data_verify, data_read, 8) != 0)
    {
      result = 0;
    }

  }
  Mem_Init();
  {
    uint8_t data [] = {0xff,0xee,0xee,0xdd};
    uint8_t data_verify [] = {0xff,0xee,0xee,0xdd};
    Mem_Write(BASE_ADDR_SPI + ADDR_2, 4, data, 4);

    uint8_t data_read [4] = {0};
    Mem_Read(BASE_ADDR_SPI + ADDR_2, 4, data_read, 4);

    if(memcmp(data_verify, data_read, 4) != 0)
    {
      result = 0;
    }

  }
  return result;
}
