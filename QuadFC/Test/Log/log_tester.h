/*
 * Log_tester.h
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
#ifndef TEST_LOG_LOG_TESTER_H_
#define TEST_LOG_LOG_TESTER_H_

#include "Test/TestFW/test_framework.h"

/**
 * Get all testcases and do needed initializations.
 * @param obj    test fw object.
 */
void Log_GetTCs(TestFw_t* obj);

uint8_t LogHandler_TestCreate(TestFw_t* obj);

uint8_t Log_TestCreate(TestFw_t* obj);

uint8_t Log_TestSetChild(TestFw_t* obj);

uint8_t Log_TestReport(TestFw_t* obj);

uint8_t Log_TestMultipleLoggers(TestFw_t* obj);

uint8_t Log_TestMultipleHandlers(TestFw_t* obj);

uint8_t Log_TestGetName(TestFw_t* obj);

uint8_t Log_TestGetNameSerialized(TestFw_t* obj);

#endif /* TEST_LOG_LOG_TESTER_H_ */
