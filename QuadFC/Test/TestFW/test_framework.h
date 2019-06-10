/*
 * TestFramework.h
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
#ifndef TEST_TESTFW_TEST_FRAMEWORK_H_
#define TEST_TESTFW_TEST_FRAMEWORK_H_
#include <stdint.h>

/**
 * Internal structure of TestFW is private. Use this type as a handle.
 */
typedef struct TestFwInternal TestFw_t;

/**
 * @brief the type of functions used in the test framework.
 *
 * All test cases register themselves with the test framework. Registering
 * a test function makes sure that it is run in the test suite.
 * @param   Pointer test fw object.
 * @return  1 if successful test, 0 otherwise.
 */
typedef uint8_t (*TestFW_TestFunction_t)(TestFw_t*);

/**
 * @brief function implementing a task has this signature.
 *
 * @param pvParameters  pointer to task parameters.
 */
typedef void (*TestFW_mainTestFunction_t)(void *pvParameters);

/**
 * Create main test task. Only to be used if the test suite uses
 * multiple threads.
 */
void TestFW_CreateMainTestTask();


/**
 * Create an instance of the test framework.
 * @return  pointer to the instance.
 */
TestFw_t* TestFW_Create();

/**
 * @brief Register a test case with the test framework.
 *
 * All test cases has to be registered with the test framework to be
 * executed.
 * @param obj           Test framework object to register with.
 * @param nameTC        Name of the test case. Max 16 bytes including null-termination.
 * @param testfcn       The function executing the test.
 * @return              1 if successfully registered, 0 otherwise.
 */
uint8_t TestFW_RegisterTest(TestFw_t* obj,const char* nameTC, TestFW_TestFunction_t testfcn);

/**
 * Set the test suite internal data.
 * @param obj           Test framework object.
 * @param ptr           Pointer to test suite data.
 * @param idx           Index in the array of internal data.
 */
void*  TestFW_SetTestSuiteInternal(TestFw_t* obj, void* ptr, uint8_t idx);

/**
 * Get the test suite internal data.
 * @param obj           Test framework object.
 * @param idx           Index in the array of internal data.
 * @return              Pointer to test suite internal data.
 */
void* TestFW_GetTestSuiteInternal(TestFw_t* obj, uint8_t idx);

/**
 *
 * @param obj     The testFW object.
 * @param string  Null terminated string.
 * @return        1 if ok, 0 otherwise.
 */
uint8_t TestFW_Report(TestFw_t* obj, const char* string);

/**
 * @brief Execute all registered tests.
 * @return
 */
uint8_t TestFW_ExecuteTests(TestFw_t* obj);


#endif /* TEST_TESTFW_TEST_FRAMEWORK_H_ */
