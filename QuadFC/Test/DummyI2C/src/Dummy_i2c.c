/*
 * Dummy_i2c.c
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
#include "Test/DummyI2C/Dummy_i2c.h"
#include <string.h>

#define MAX_NR_ENTRIES (50)
static uint32_t      twi_Init[]      = {0, 0};
static QuadFC_I2C_t* dataExpected[MAX_NR_ENTRIES] = {0};
static QuadFC_I2C_t* dataResponse[MAX_NR_ENTRIES] = {0};
static uint32_t      nrUsage[MAX_NR_ENTRIES] = {0};

static uint32_t      store_idx = 0;
static uint32_t      read_idx = 0;
static uint32_t      usedNr = 0;

uint8_t Init_Twi(int index)
{
  if(index > (sizeof(twi_Init)/sizeof(twi_Init[0])))
  {
    return 0;
  }
  if(twi_Init[index] == 1)
  {
    return 1; // already initialized.
  }
  twi_Init[index] = 1;
  return 1;
}


uint8_t QuadFC_i2cWrite(QuadFC_I2C_t *i2c_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  (void)blockTimeMs;
  return DummyI2C_ReadWrite(i2c_data, busIndex, 0);
}

uint8_t QuadFC_i2cRead(QuadFC_I2C_t *i2c_data, uint8_t busIndex, TickType_t blockTimeMs)
{
  (void)blockTimeMs;
  return DummyI2C_ReadWrite(i2c_data, busIndex, 1);
}

uint8_t DummyI2C_ReadWrite(QuadFC_I2C_t *i2c_data, uint8_t busIndex, uint8_t isRead)
{
  if(busIndex > (sizeof(twi_Init)/sizeof(twi_Init[0])))
  {
    return 0;
  }
  if(!twi_Init[busIndex])
  {
    if(!Init_Twi(busIndex))
    {
      return 0;
    }
  }

  // Read idx has to be smaller than store idx.
  if(read_idx >= store_idx)
  {
    return 0;
  }
  uint8_t result = 0;
  if(dataExpected[read_idx])
  {
    if(isRead)
    {
      if(DummyI2C_Compare(i2c_data, dataExpected[read_idx], 0))
      {
        //We got what we expected, replay.
        DummyI2C_Populate(i2c_data, dataResponse[read_idx]);

        result = 1;
      }
    }
    else
    {
      if(DummyI2C_Compare(i2c_data, dataExpected[read_idx], 1))
      {
        //We got what we expected! No action needed for write.
        result = 1;
      }
    }
  }
  else // no expected request, answer anyway.
  {
    if(isRead)
    {
      //replay.
      DummyI2C_Populate(i2c_data, dataResponse[read_idx]);
    }
    result = 1;
  }
  if(++usedNr >= nrUsage[read_idx])
  {
    usedNr = 0;
    if(dataExpected[read_idx] != NULL)
    {
      free(dataExpected[read_idx]);
    }
    if(dataResponse[read_idx] != NULL)
    {
      free(dataResponse[read_idx]);
    }
    read_idx++;
  }
  return result;
}

uint8_t DummyI2C_AddResponse(int NrUsage, QuadFC_I2C_t* expectedInput, QuadFC_I2C_t* response)
{
  if(store_idx >= MAX_NR_ENTRIES)
  {
    return 0;
  }
  dataExpected[store_idx]   = expectedInput;
  dataResponse[store_idx] = response;
  nrUsage[store_idx++] = NrUsage;
  return 1;
}

uint8_t DummyI2C_Compare(const QuadFC_I2C_t* first, const QuadFC_I2C_t* second, uint8_t deepcmp)
{
  // First shallow compare of all fields that are not pointers.
  if(
      first->internalAddrLength == second->internalAddrLength &&
      first->bufferLength == second->bufferLength &&
      first->slaveAddress == second->slaveAddress)
  {
    for(int i = 0; i < second->internalAddrLength; i++)
    {
      if(first->internalAddr[i] != second->internalAddr[i])
      {
        return 0;
      }
    }
    //If OK, then do deep compare of the pointer fields.
    if(deepcmp)
    {
      int is_equal = memcmp(first->buffer, second->buffer, second->bufferLength);
      if(is_equal == 0) // memcmp returns 0 on equality.
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }
  else
  {
    return 0;
  }
  return 1;
}

uint8_t DummyI2C_Populate(const QuadFC_I2C_t* target, const QuadFC_I2C_t* reference)
{
  if( target->bufferLength == reference->bufferLength )
  {
    //If OK, then do deep compare of the pointer fields.
    memcpy(target->buffer, reference->buffer, reference->bufferLength);
  }
  return 1;
}





