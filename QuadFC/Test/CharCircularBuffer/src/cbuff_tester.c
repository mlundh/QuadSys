/*
 * cbuff_tester.c
 *
 * Copyright (C) 2020 Martin Lundh
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

#include "Test/CharCircularBuffer/cbuff_tester.h"
#include "Components/CharCircularBuffer/inc/CharCircularBuffer.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

uint8_t Cbuff_TestCreateDelete(TestFw_t *obj);
uint8_t Cbuff_TestPushPop(TestFw_t *obj);
uint8_t Cbuff_TestCircular(TestFw_t *obj);
uint8_t Cbuff_TestPushPopCirc(TestFw_t *obj);
uint8_t Cbuff_TestFull(TestFw_t *obj);
uint8_t Cbuff_TestPushPopString(TestFw_t *obj);
uint8_t Cbuff_TestPushPopStringBoundary(TestFw_t *obj);
uint8_t Cbuff_TestReadMore(TestFw_t *obj);

void Cbuff_GetTCs(TestFw_t *obj)
{
  TestFW_RegisterTest(obj, "Cbuff Create Delete", Cbuff_TestCreateDelete);
  TestFW_RegisterTest(obj, "Cbuff Push Pop", Cbuff_TestPushPop);
  TestFW_RegisterTest(obj, "Cbuff Circular", Cbuff_TestCircular);
  TestFW_RegisterTest(obj, "Cbuff Count", Cbuff_TestPushPopCirc);
  TestFW_RegisterTest(obj, "Cbuff Full", Cbuff_TestFull);
  TestFW_RegisterTest(obj, "Cbuff Push Pop String", Cbuff_TestPushPopString);
  TestFW_RegisterTest(obj, "Cbuff Push Pop String boundary", Cbuff_TestPushPopStringBoundary);
  TestFW_RegisterTest(obj, "Cbuff Read more than is availible", Cbuff_TestReadMore);
}

uint8_t Cbuff_TestCreateDelete(TestFw_t *obj)
{
  CharCircBuffer_t *cbuff = CharCircBuff_Create(255);
  CharCircBuff_Delete(cbuff);
  return 1;
}

#define BUFFERSIZE (254) // must be even and less than 256 for these tests. 
uint8_t Cbuff_TestPushPop(TestFw_t *obj)
{
  uint32_t result = 1;
  CharCircBuffer_t *cbuff = CharCircBuff_Create(BUFFERSIZE);
  // fill all elements.
  for (uint8_t i = 0; i < BUFFERSIZE; i++)
  {
    CharCircBuff_Push(cbuff, &i, 1);
  }

  uint8_t buffer[BUFFERSIZE] = {0};
  uint32_t size = CharCircBuff_Pop(cbuff, buffer, BUFFERSIZE);

  if (size != BUFFERSIZE)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Expected to pop %d got %ld.", BUFFERSIZE, size);
  }
  for (uint32_t i = 0; i < BUFFERSIZE; i++)
  {
    if (buffer[i] != i)
    {
      TEST_REPORT_ERROR(obj, "Expected: %ld Got: %d", i, buffer[i]);
      result = 0;
    }
  }
  CharCircBuff_Delete(cbuff);
  return result;
}

uint8_t Cbuff_TestCircular(TestFw_t *obj)
{
  uint32_t result = 1;
  CharCircBuffer_t *cbuff = CharCircBuff_Create(BUFFERSIZE);
  // fill all elements.
  TEST_REPORT_INFO(obj, "Filling the whole buffer.");
  for (uint8_t i = 0; i < BUFFERSIZE; i++)
  {
    CharCircBuff_Push(cbuff, &i, 1);
  }
  uint32_t nrElements = CharCircBuff_NrElemets(cbuff);
  if (nrElements != BUFFERSIZE)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Unexpected number of elements.");
  }
  {
    TEST_REPORT_INFO(obj, "Read half of the buffer.");
    // Read and verify half
    uint8_t buffer[BUFFERSIZE / 2] = {0};
    uint32_t size = CharCircBuff_Pop(cbuff, buffer, BUFFERSIZE / 2);

    if (size != BUFFERSIZE / 2)
    {
      result = 0;
      TEST_REPORT_ERROR(obj, "Expected to pop %d got %ld.", BUFFERSIZE / 2, size);
    }
    nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != BUFFERSIZE / 2)
    {
      result = 0;
      TEST_REPORT_ERROR(obj, "Unexpected number of elements.");
    }

    for (uint8_t i = 0; i < BUFFERSIZE / 2; i++)
    {
      if (buffer[i] != i)
      {
        TEST_REPORT_ERROR(obj, "Expected: %d Got: %d", i, buffer[i]);
        result = 0;
      }
    }
  }
  TEST_REPORT_INFO(obj, "Re-fill the buffer.");
  // re-fill
  for (uint8_t i = 0; i < BUFFERSIZE / 2; i++)
  {
    if (!CharCircBuff_Push(cbuff, &i, 1))
    {
      TEST_REPORT_ERROR(obj, "Not able to push: %d.", i);
      result = 0;
    }
  }
  nrElements = CharCircBuff_NrElemets(cbuff);
  if (nrElements != BUFFERSIZE)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Unexpected number of elements.");
  }
  {
    TEST_REPORT_INFO(obj, "Read the whole buffer.");
    // Read and verify the rest. This read will cross the bondary.
    uint8_t buffer[BUFFERSIZE] = {0};
    uint32_t size = CharCircBuff_Pop(cbuff, buffer, BUFFERSIZE);

    if (size != BUFFERSIZE)
    {
      result = 0;
      TEST_REPORT_ERROR(obj, "ERROR: Expected to pop %d got %ld.", BUFFERSIZE, size);
    }
    TEST_REPORT_INFO(obj, "Verify first half.");
    for (uint8_t i = 0; i < BUFFERSIZE / 2; i++)
    {
      if (buffer[i] != i + (BUFFERSIZE / 2))
      {
        TEST_REPORT_ERROR(obj, "Expected: %d Got: %d", i + (BUFFERSIZE / 2), buffer[i]);
        result = 0;
      }
    }
    TEST_REPORT_INFO(obj, "Verify second half.");
    for (uint8_t i = 0; i < BUFFERSIZE / 2 - 1; i++)
    {
      if (buffer[i + (BUFFERSIZE / 2)] != i)
      {
        TEST_REPORT_ERROR(obj, "Expected: %d Got: %d", i, buffer[i + (BUFFERSIZE / 2)]);
        result = 0;
      }
    }
  }
  CharCircBuff_Delete(cbuff);
  return result;
}

uint8_t Cbuff_TestPushPopCirc(TestFw_t *obj)
{
  uint32_t result = 1;
  CharCircBuffer_t *cbuff = CharCircBuff_Create(BUFFERSIZE);
  // fill all elements.
  for (uint32_t i = 0; i < BUFFERSIZE * 3; i++)
  {
    uint8_t data = i % BUFFERSIZE;
    CharCircBuff_Push(cbuff, &data, 1);
    uint32_t nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != 1)
    {
      TEST_REPORT_ERROR(obj, "Unexpected number of elements.");
    }

    uint8_t buffer = 0;
    CharCircBuff_Pop(cbuff, &buffer, 1);
    if (buffer != data)
    {
      result = 0;
      TEST_REPORT_ERROR(obj, "Expected: %d Got: %d", data, buffer);
    }
    nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != 0)
    {
      TEST_REPORT_ERROR(obj, "Unexpected number of elements.");
    }
  }
  for (uint32_t i = BUFFERSIZE * 3; i > 0; i--)
  {
    uint8_t data = i % BUFFERSIZE;
    CharCircBuff_Push(cbuff, &data, 1);
    uint32_t nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != 1)
    {
      TEST_REPORT_ERROR(obj, "Unexpected number of elements.");
    }

    uint8_t buffer = 0;
    CharCircBuff_Pop(cbuff, &buffer, 1);
    if (buffer != data)
    {
      result = 0;
      TEST_REPORT_ERROR(obj, "Expected: %d Got: %d", data, buffer);
    }
    nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != 0)
    {
      TEST_REPORT_ERROR(obj, "Unexpected number of elements.");
    }
  }
  CharCircBuff_Delete(cbuff);
  return result;
}

uint8_t Cbuff_TestFull(TestFw_t *obj)
{
  uint32_t result = 1;
  CharCircBuffer_t *cbuff = CharCircBuff_Create(BUFFERSIZE);
  // fill all elements.
  for (uint8_t i = 0; i < BUFFERSIZE; i++)
  {
    uint32_t pushSuccess = CharCircBuff_Push(cbuff, &i, 1);
    if (!pushSuccess)
    {
      result = 0;
      TEST_REPORT_ERROR(obj, "Unable to push.");
    }
    uint32_t nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != i + 1)
    {
      result = 0;
      TEST_REPORT_ERROR(obj, "Unexpected number of elements, expected: %d Got: %ld", i, nrElements);
    }
  }
  uint8_t data = 222;
  uint8_t pushSuccess = CharCircBuff_Push(cbuff, &data, 1);
  ;
  if (pushSuccess != 0)
  {
    TEST_REPORT_ERROR(obj, "Able to push to full buffer.");
  }

  CharCircBuff_Delete(cbuff);
  return result;
}

uint8_t Cbuff_TestPushPopString(TestFw_t *obj)
{
  uint32_t result = 1;
  CharCircBuffer_t *cbuff = CharCircBuff_Create(BUFFERSIZE);
  // fill all elements.
  uint8_t data[BUFFERSIZE];
  for (uint32_t i = 0; i < BUFFERSIZE; i++)
  {
    data[i] = i;
  }
  uint32_t size = CharCircBuff_Push(cbuff, data, BUFFERSIZE);
  if(size != BUFFERSIZE)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Expected to push %d got %ld.", BUFFERSIZE, size);
  }

  uint8_t buffer[BUFFERSIZE] = {0};
  size = CharCircBuff_Pop(cbuff, buffer, BUFFERSIZE);

  if (size != BUFFERSIZE)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Expected to pop %d got %ld.", BUFFERSIZE, size);
  }
  for (uint32_t i = 0; i < BUFFERSIZE; i++)
  {
    if (buffer[i] != data[i])
    {
      TEST_REPORT_ERROR(obj, "Expected: %d Got: %d", data[i], buffer[i]);
      result = 0;
    }
  }
  CharCircBuff_Delete(cbuff);
  return result;
}

uint8_t Cbuff_TestPushPopStringBoundary(TestFw_t *obj)
{
  uint32_t result = 1;
  CharCircBuffer_t *cbuff = CharCircBuff_Create(BUFFERSIZE);
  // fill all elements.
  uint8_t data[BUFFERSIZE];
  for (uint32_t i = 0; i < BUFFERSIZE; i++)
  {
    data[i] = i;
  }
  TEST_REPORT_INFO(obj, "Filling the whole buffer.");
  uint32_t size = CharCircBuff_Push(cbuff, data, BUFFERSIZE);
  if (size != BUFFERSIZE)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Expected to push %d got %ld.", BUFFERSIZE, size);
  }

  TEST_REPORT_INFO(obj, "Reading half the buffer.");
  uint8_t buffer[BUFFERSIZE/2] = {0};
  size = CharCircBuff_Pop(cbuff, buffer, BUFFERSIZE/2);

  if (size != BUFFERSIZE/2)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Expected to pop %d got %ld.", BUFFERSIZE, size);
  }
  for (uint32_t i = 0; i < BUFFERSIZE/2; i++)
  {
    if (buffer[i] != data[i])
    {
      TEST_REPORT_ERROR(obj, "Expected: %d Got: %d", data[i], buffer[i]);
      result = 0;
    }
  }


  TEST_REPORT_INFO(obj, "re-filling the buffer.");
  size = CharCircBuff_Push(cbuff, data, BUFFERSIZE/2); // data is unchanged. re-use.
  if (size != BUFFERSIZE/2)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Expected to push %d got %ld.", BUFFERSIZE/2, size);
  }
  TEST_REPORT_INFO(obj, "Reading the complete buffer.");

  uint8_t bufferExpected[BUFFERSIZE] = {0};
  for (uint32_t i = 0; i < BUFFERSIZE/2; i++)
  {
    bufferExpected[i] = i + (BUFFERSIZE/2);
  }
  for (uint32_t i = BUFFERSIZE / 2; i < BUFFERSIZE; i++)
  {
    bufferExpected[i] = i - (BUFFERSIZE / 2);
  }

  uint8_t bufferVerify[BUFFERSIZE] = {0};
  size = CharCircBuff_Pop(cbuff, bufferVerify, BUFFERSIZE);

  for (uint32_t i = 0; i < BUFFERSIZE; i++)
  {
    if (bufferVerify[i] != bufferExpected[i])
    {
      TEST_REPORT_ERROR(obj, "Expected: %d Got: %d at index %ld", bufferExpected[i], bufferVerify[i], i);
      result = 0;
    }
  }

  CharCircBuff_Delete(cbuff);
  return result;
}

uint8_t Cbuff_TestReadMore(TestFw_t *obj)
{
  uint32_t result = 1;
  CharCircBuffer_t *cbuff = CharCircBuff_Create(BUFFERSIZE);
  // fill all elements.
  uint8_t data[BUFFERSIZE];
  for (uint32_t i = 0; i < BUFFERSIZE/2; i++)
  {
    data[i] = i;
  }
  uint32_t size = CharCircBuff_Push(cbuff, data, BUFFERSIZE);
  if (size != BUFFERSIZE)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Expected to push %d got %ld.", BUFFERSIZE, size);
  }

  uint8_t buffer[BUFFERSIZE] = {0};
  size = CharCircBuff_Pop(cbuff, buffer, BUFFERSIZE/2);

  if (size != BUFFERSIZE/2)
  {
    result = 0;
    TEST_REPORT_ERROR(obj, "Expected to pop %d got %ld.", BUFFERSIZE/2, size);
  }
  for (uint32_t i = 0; i < BUFFERSIZE/2; i++)
  {
    if (buffer[i] != data[i])
    {
      TEST_REPORT_ERROR(obj, "Expected: %d Got: %d", data[i], buffer[i]);
      result = 0;
    }
  }
  CharCircBuff_Delete(cbuff);
  return result;
}