/*
 * utilities_tester.c
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

#include "Utilities/inc/serialization.h"
#include "Test/Utilities/utilities_tester.h"
#include "Utilities/inc/string_utils.h"
#include "Log/inc/logHandler.h"

#include <string.h>

uint8_t Utilities_Testserialization(TestFw_t* obj);
uint8_t Utilities_TestStringSerialization(TestFw_t* obj);
uint8_t Utilities_TestUsigtSerialization(TestFw_t* obj);


void Utilities_GetTCs(TestFw_t* obj)
{
  TestFW_RegisterTest(obj, "uint Seri", Utilities_Testserialization);
  TestFW_RegisterTest(obj, "string Seri", Utilities_TestStringSerialization);
  TestFW_RegisterTest(obj, "U sig Seri", Utilities_TestUsigtSerialization);


}
#define UTILITIES_TEST_BUFF_SIZE 12
uint8_t Utilities_Testserialization(TestFw_t* obj)
{
  uint8_t buffer[UTILITIES_TEST_BUFF_SIZE] = {0};
  logEntry_t entry;
  entry.data = 555;
  entry.id = 12;
  entry.time = 8888;

  uint32_t buffsize = UTILITIES_TEST_BUFF_SIZE;

  uint8_t* bufferP = &buffer[0];
  bufferP = serialize_int32_t(bufferP, &buffsize, (int32_t*)(&entry.data));
  bufferP = serialize_int32_t(bufferP, &buffsize, (int32_t*)(&entry.id));
  bufferP = serialize_int32_t(bufferP, &buffsize, (int32_t*)(&entry.time));

  logEntry_t entry_verify = {0};

  uint32_t buffsizeRead = UTILITIES_TEST_BUFF_SIZE;
  uint8_t* bufferPread = &buffer[0];

  bufferPread = deserialize_int32_t(bufferPread, &buffsizeRead, (int32_t*)&(entry_verify.data));
  bufferPread = deserialize_int32_t(bufferPread, &buffsizeRead, (int32_t*)&(entry_verify.id));
  bufferPread = deserialize_int32_t(bufferPread, &buffsizeRead, (int32_t*)&(entry_verify.time));

  if(entry.data == entry_verify.data &&
      entry.id == entry_verify.id &&
      entry.time == entry_verify.time)
  {
    return 1;
  }
  return 0;
}
#undef UTILITIES_TEST_BUFF_SIZE

#define UTILITIES_TEST_STRING_BUFF_SIZE 120
uint8_t Utilities_TestStringSerialization(TestFw_t* obj)
{

  uint8_t string[UTILITIES_TEST_STRING_BUFF_SIZE] = {"TestString: testing\0"};
  uint8_t* stringP = &string[0];
  uint32_t stringLength = strlen((char*)stringP);

  uint8_t buffer[UTILITIES_TEST_STRING_BUFF_SIZE] = {0};
  uint32_t buffer_size_left = UTILITIES_TEST_STRING_BUFF_SIZE;
  uint8_t* bufferPtr = &buffer[0];

  bufferPtr = serialize_string(bufferPtr, &buffer_size_left, stringP, stringLength);


  uint32_t buffsizeRead = UTILITIES_TEST_STRING_BUFF_SIZE;
  uint8_t* bufferPread = &buffer[0];

  uint8_t stringVerify[UTILITIES_TEST_STRING_BUFF_SIZE] = {0};
  uint32_t stringLengthVerify = 0;

  bufferPread = deserialize_string(bufferPread, &buffsizeRead, stringVerify, &stringLengthVerify, UTILITIES_TEST_STRING_BUFF_SIZE);




  if((stringLength == stringLengthVerify) && (strncmp((char*)stringVerify, (char*)string, stringLength) == 0))
  {
    return 1;
  }
  return 0;
}
#undef UTILITIES_TEST_STRING_BUFF_SIZE



#define UTILITIES_TEST_U_SIGNAL_BUFF_SIZE 120
uint8_t Utilities_TestUsigtSerialization(TestFw_t* obj)
{
  uint8_t buffer[UTILITIES_TEST_U_SIGNAL_BUFF_SIZE] = {0};
  control_signal_t entry = {{1,2,3,4}};

  uint32_t buffer_size_left = UTILITIES_TEST_U_SIGNAL_BUFF_SIZE;
  uint8_t* bufferPtr = &buffer[0];
  bufferPtr = serialize_control_signal_t(bufferPtr, &buffer_size_left, &entry);

  control_signal_t entryVerify = {0};

  uint32_t buffsizeRead = UTILITIES_TEST_U_SIGNAL_BUFF_SIZE;
  uint8_t* bufferPread = &buffer[0];

  deserialize_control_signal_t(bufferPread, &buffsizeRead, &entryVerify);

  uint8_t result = 1;
  for(int i = 0; i < 4; i++)
  {
      if(entryVerify.control_signal[i] != entry.control_signal[i])
      {
          result = 0;
      }
  }

  return result;
}
#undef UTILITIES_TEST_U_SIGNAL_BUFF_SIZE
