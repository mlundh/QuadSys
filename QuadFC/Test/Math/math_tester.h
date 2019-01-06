/*
 * MathTester.h
 *
 * Copyright (C) 2016 Martin Lundh
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
#ifndef TEST_MATHTESTER_H_
#define TEST_MATHTESTER_H_

#include "Test/TestFW/test_framework.h"

void MathTest_GetTCs(TestFw_t* obj);

uint8_t MathTest_TestTan2(TestFw_t* obj);

uint8_t MathTest_TestSqrt(TestFw_t* obj);

/**
 * Evaluate the Sqrt implementation.
 *
 * Use a logic level analyzer or a oscilloscope to determine
 * the time spent.
 *
 * Pin 39 determines the time it takes for ref (sqrt) implementation.
 * Pin 37 determines the time it takes for custom implementation.
 *
 * Signal goes high:
 *    start
 * Signal goes low:
 *    end.
 *
 * With current implementation the custom is approximately
 * 2.6 times faster than the reference on an ARM Cortex-M3
 * without floating point unit.
 *
 * @param obj
 * @return
 */
uint8_t MathTest_TestSqrtPerformance(TestFw_t* obj);


#endif /* TEST_MATHTESTER_H_ */
