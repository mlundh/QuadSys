/*
 * log_test_backend.c
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

#include "Log/inc/log_backend.h"
#include "Log/inc/logHandler.h"
#include "FreeRTOS.h"
#include "task.h"

#define TEST_LOG_LENGTH 5
struct logBackEnd
{
  logEntry_t report[TEST_LOG_LENGTH];
  uint8_t idx;
  uint8_t read;
};

LogBackEnd_t* LogBackend_CreateObj()
{
  LogBackEnd_t* obj = pvPortMalloc(sizeof(LogBackEnd_t));
  if(!obj)
  {
    return NULL;
  }
  obj->idx = 0;
  obj->read = 0;
  return obj;
}

void LogBackend_DeleteObj(LogBackEnd_t* obj)
{
    if(!obj)
    {
        return;
    }
    vPortFree(obj);
}

uint8_t LogBackend_Report(LogBackEnd_t* obj, logEntry_t* entry)
{
  if(!obj)
  {
    return 0;;
  }

  obj->report[obj->idx].data = entry->data;
  obj->report[obj->idx].id = entry->id;
  obj->report[obj->idx].time = entry->time;
  obj->idx++;
  obj->idx %= TEST_LOG_LENGTH;
  return 1;
}

uint8_t LogBackend_GetLog(LogBackEnd_t* obj, logEntry_t* entries, uint32_t size, uint32_t* nrLogs)
{
  int j = 0;
  for(; (obj->read < size) && (obj->read < obj->idx); obj->read++)
  {
    entries[j++] = obj->report[obj->read];
  }
  *nrLogs = j;
  return 1;
}

