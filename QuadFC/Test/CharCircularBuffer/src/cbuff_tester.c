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

void Cbuff_GetTCs(TestFw_t *obj)
{
  TestFW_RegisterTest(obj, "Cbuff Create Delete", Cbuff_TestCreateDelete);
  TestFW_RegisterTest(obj, "Cbuff Push Pop", Cbuff_TestPushPop);
  TestFW_RegisterTest(obj, "Cbuff Circular", Cbuff_TestCircular);
  TestFW_RegisterTest(obj, "Cbuff Count", Cbuff_TestPushPopCirc);
  TestFW_RegisterTest(obj, "Cbuff Full", Cbuff_TestFull);
}

uint8_t Cbuff_TestCreateDelete(TestFw_t *obj)
{
  CharCircBuffer_t *cbuff = CharCircBuff_Create(255);
  CharCircBuff_Delete(cbuff);
  return 1;
}

#define BUFFERSIZE (256)
uint8_t Cbuff_TestPushPop(TestFw_t *obj)
{
  uint32_t result = 1;
  CharCircBuffer_t *cbuff = CharCircBuff_Create(BUFFERSIZE);
  // fill all elements.
  for (uint32_t i = 0; i < BUFFERSIZE; i++)
  {
    CharCircBuff_Push(cbuff, i);
  }

  uint8_t buffer[BUFFERSIZE] = {0};
  uint32_t size = CharCircBuff_Pop(cbuff, buffer, BUFFERSIZE);

  if (size != BUFFERSIZE)
  {
    result = 0;
    TEST_REPORT_ENTRY(obj, "ERROR: Expected to pop %d got %ld.\n", BUFFERSIZE, size);
  }
  for (uint32_t i = 0; i < BUFFERSIZE; i++)
  {
    if (buffer[i] != i)
    {
      TEST_REPORT_ENTRY(obj, "ERROR: Expected: %ld Got: %d\n", i, buffer[i]);
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
  TestFW_Report(obj, "Filling the whole buffer.\n");
  for (uint32_t i = 0; i < BUFFERSIZE; i++)
  {
    CharCircBuff_Push(cbuff, i);
  }
  uint32_t nrElements = CharCircBuff_NrElemets(cbuff);
  if (nrElements != BUFFERSIZE)
  {
    result = 0;
    TestFW_Report(obj, "ERROR: Unexpected number of elements.\n");
  }
  {
    TestFW_Report(obj, "Read half of the buffer.\n");
    // Read and verify half
    uint8_t buffer[BUFFERSIZE / 2] = {0};
    uint32_t size = CharCircBuff_Pop(cbuff, buffer, BUFFERSIZE / 2);

    if (size != BUFFERSIZE / 2)
    {
      result = 0;
      TEST_REPORT_ENTRY(obj, "ERROR: Expected to pop %d got %ld.\n", BUFFERSIZE / 2, size);
    }
    nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != BUFFERSIZE / 2)
    {
      result = 0;
      TestFW_Report(obj, "ERROR: Unexpected number of elements.\n");
    }

    for (uint8_t i = 0; i < BUFFERSIZE / 2; i++)
    {
      if (buffer[i] != i)
      {
        TEST_REPORT_ENTRY(obj, "ERROR: Expected: %d Got: %d\n", i, buffer[i]);
        result = 0;
      }
    }
  }
  TestFW_Report(obj, "Re-fill the buffer.\n");
  // re-fill
  for (uint32_t i = 0; i < BUFFERSIZE / 2; i++)
  {
    if (!CharCircBuff_Push(cbuff, i))
    {
      TestFW_Report(obj, "ERROR: Not able to push: %ld.\n");
      result = 0;
    }
  }
  nrElements = CharCircBuff_NrElemets(cbuff);
  if (nrElements != BUFFERSIZE)
  {
    result = 0;
    TestFW_Report(obj, "ERROR: Unexpected number of elements.\n");
  }
  {
    TestFW_Report(obj, "Read the whole buffer.\n");
    // Read and verify the rest. This read will cross the bondary.
    uint8_t buffer[BUFFERSIZE] = {0};
    uint32_t size = CharCircBuff_Pop(cbuff, buffer, BUFFERSIZE);

    if (size != BUFFERSIZE)
    {
      result = 0;
      TEST_REPORT_ENTRY(obj, "ERROR: Expected to pop %d got %ld.\n", BUFFERSIZE, size);
    }
    TestFW_Report(obj, "Verify first half.\n");
    for (uint8_t i = 0; i < BUFFERSIZE / 2; i++)
    {
      if (buffer[i] != i + (BUFFERSIZE / 2))
      {
        TEST_REPORT_ENTRY(obj, "ERROR: Expected: %d Got: %d\n", i + (BUFFERSIZE / 2), buffer[i]);
        result = 0;
      }
    }
    TEST_REPORT_ENTRY(obj, "Verify second half.\n");
    for (uint8_t i = 0; i < BUFFERSIZE / 2 - 1; i++)
    {
      if (buffer[i + (BUFFERSIZE / 2)] != i)
      {
        TEST_REPORT_ENTRY(obj, "ERROR: Expected: %d Got: %d\n", i, buffer[i + (BUFFERSIZE / 2)]);
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
    CharCircBuff_Push(cbuff, data);
    uint32_t nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != 1)
    {
      TestFW_Report(obj, "ERROR: Unexpected number of elements.\n");
    }

    uint8_t buffer = 0;
    CharCircBuff_Pop(cbuff, &buffer, 1);
    if (buffer != data)
    {
      result = 0;
      TEST_REPORT_ENTRY(obj, "ERROR: Expected: %d Got: %d\n", data, buffer);
    }
    nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != 0)
    {
      TestFW_Report(obj, "ERROR: Unexpected number of elements.\n");
    }
  }
  for (uint32_t i = BUFFERSIZE * 3; i > 0; i--)
  {
    uint8_t data = i % BUFFERSIZE;
    CharCircBuff_Push(cbuff, data);
    uint32_t nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != 1)
    {
      TestFW_Report(obj, "ERROR: Unexpected number of elements.\n");
    }

    uint8_t buffer = 0;
    CharCircBuff_Pop(cbuff, &buffer, 1);
    if (buffer != data)
    {
      result = 0;
      TEST_REPORT_ENTRY(obj, "ERROR: Expected: %d Got: %d\n", data, buffer);
    }
    nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != 0)
    {
      TestFW_Report(obj, "ERROR: Unexpected number of elements.\n");
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
  for (uint32_t i = 0; i < BUFFERSIZE; i++)
  {
    uint8_t pushSuccess = CharCircBuff_Push(cbuff, i);
    if (!pushSuccess)
    {
      result = 0;
      TestFW_Report(obj, "ERROR: Unable to push.\n");
    }
    uint32_t nrElements = CharCircBuff_NrElemets(cbuff);
    if (nrElements != i + 1)
    {
      result = 0;
      TEST_REPORT_ENTRY(obj, "ERROR: Unexpected number of elements, expected: %ld Got: %ld\n", i, nrElements);
    }
  }
  uint8_t pushSuccess = CharCircBuff_Push(cbuff, 222);
  if (pushSuccess != 0)
  {
    TestFW_Report(obj, "ERROR: Able to push to full buffer.\n");
  }

  CharCircBuff_Delete(cbuff);
  return result;
}