/*
 * TestFramework.c
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
#include "Test/TestFW/test_framework.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "HAL/QuadFC/QuadFC_Serial.h"

#include <gpio.h>


#define MAX_NR_TEST_FUNCTIONS (64)
#define MAX_NR_INTERNALS (16)
#define REPORT_SIZE (0x4000)
#define TC_Name_LENGTH (32)
#define REPORT_ENTRY_MAX (64)

typedef struct TestCase
{
    TestFW_TestFunction_t testFcn;
    char tc_names[TC_Name_LENGTH];
    uint8_t testOK;

} TC_t;

struct TestFwInternal
{
    uint32_t nrRegisteredFunctions;
    TC_t testFcns[MAX_NR_TEST_FUNCTIONS];
    char name[TC_Name_LENGTH];
    char report[REPORT_SIZE];
    void* testSuiteInternal[MAX_NR_INTERNALS];
};

/**
 * Get the test report. The header will contain the string "ALL PASS" if
 * all tests passed, and the word "FAIL" if any test failed.
 *
 * The report will specify which tests was run, and which passed. Each test
 * that was run will have its own result, and a field to write a short error
 * message.
 *
 * @return A pointer to the report string. The string is null-terminated.
 */
uint8_t* TestFW_GetReport(TestFw_t* obj);


void TestFW_CreateMainTestTask(TestFW_mainTestFunction_t mainFcn)
{
    /*Create the task*/

    portBASE_TYPE create_result;
    create_result = xTaskCreate( mainFcn,             /* The function that implements the task.  */
            (const char *const) "TestTask" ,              /* The name of the task. This is not used by the kernel, only aids in debugging*/
            700,                                          /* The stack size for the task*/
            NULL,                                         /* pass params to task.*/
            configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
            NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

    if ( create_result != pdTRUE )
    {
        /*Error - Could not create task.*/
        for ( ;; )
        {

        }
    }
}


TestFw_t* TestFW_Create(char* name)
{
    Test_SerialInit();
    TestFw_t* ptr = malloc(sizeof(TestFw_t));
    ptr->nrRegisteredFunctions = 0;
    ptr->report[0] = '\0';
    strncpy( ptr->name, name, TC_Name_LENGTH);
    ptr->name[TC_Name_LENGTH-1] = '\0';
    void* tmp[MAX_NR_INTERNALS] = {NULL};
    ptr->testSuiteInternal[0] = tmp;
    return ptr;
}

uint8_t TestFW_RegisterTest(TestFw_t* obj, const char* nameTC, TestFW_TestFunction_t testfcn)
{
    if(!(obj->nrRegisteredFunctions < MAX_NR_TEST_FUNCTIONS))
    {
        return 0; // Too many registered functions!
    }
    obj->testFcns[obj->nrRegisteredFunctions].testFcn = testfcn;
    strncpy( obj->testFcns[obj->nrRegisteredFunctions].tc_names , nameTC, (unsigned short) TC_Name_LENGTH);
    obj->testFcns[obj->nrRegisteredFunctions].tc_names[TC_Name_LENGTH-1] = '\0'; // Always make sure the last char is null
    obj->nrRegisteredFunctions++;
    return 1;
}

void* TestFW_SetTestSuiteInternal(TestFw_t* obj, void* ptr, uint8_t idx)
{
    if(idx >= MAX_NR_INTERNALS)
    {
        return NULL;
    }
    obj->testSuiteInternal[idx] = ptr;
    return ptr;
}

void* TestFW_GetTestSuiteInternal(TestFw_t* obj, uint8_t idx)
{
    if(idx >= MAX_NR_INTERNALS)
    {
        return NULL;
    }
    return obj->testSuiteInternal[idx];
}

uint8_t TestFW_Report(TestFw_t* obj, const char* string)
{
    uint32_t currentReport = (uint32_t)strlen(obj->report);
    if(currentReport >= REPORT_SIZE)
    {
        return 0;
    }
    uint32_t reportLeftInBuffer = REPORT_SIZE - currentReport;
    uint32_t StringLength = (uint32_t)strlen(string) + 1; // Keep the null termination.
    uint32_t length = ((StringLength < reportLeftInBuffer) ? StringLength : reportLeftInBuffer);
    strncpy( obj->report + currentReport , string, length);
    return 1;
}

#define REPORTLEN (110)
uint8_t TestFW_ExecuteTests(TestFw_t* obj)
{
    uint8_t SuiteResult = 1;
    char tmpstr[REPORTLEN] = {0};
    snprintf(tmpstr, REPORTLEN, "\n*********************** Starting Testsuite: %s *********************** \n", obj->name);
    TestFW_Report(obj, tmpstr);
    for(int i = 0; i < obj->nrRegisteredFunctions; i++)
    {
        snprintf(tmpstr, REPORTLEN, "\n**** Starting Test: %s ****\n", obj->testFcns[i].tc_names);
        TestFW_Report(obj, tmpstr);

        // Fail if the testcase does not clean up after itself. Start measurement here.
        size_t heapSize= xPortGetFreeHeapSize();

        // Run testcase.
        obj->testFcns[i].testOK = obj->testFcns[i].testFcn(obj);

        // Evaluate the remaining heap usage.
        size_t heapSizeAfterM= xPortGetFreeHeapSize();
        size_t usedHeapM = heapSize - heapSizeAfterM;
        if(usedHeapM > 0)
        {

            char memLeakStr[REPORTLEN] = {0};
            snprintf (memLeakStr, REPORTLEN,"\e[31mMemory leak in test case %s! Lost %d bytes.\e[0m\n",obj->testFcns[i].tc_names, usedHeapM);
            TestFW_Report(obj, memLeakStr);
        }
        const char* result = (obj->testFcns[i].testOK) ? "\e[32mPASS\e[0m" : "\e[31mFAIL\e[0m";
        snprintf(tmpstr, REPORTLEN, "\n**** RESULT %s: %s **** \n" , obj->testFcns[i].tc_names, result);
        TestFW_Report(obj, tmpstr);
        TestFW_GetReport(obj);
        SuiteResult &= obj->testFcns[i].testOK;
    }
    const char* result = (SuiteResult) ? "\e[32mPASS\e[0m" : "\e[31mFAIL\e[0m";
    snprintf(tmpstr, REPORTLEN, "\n********************** Testsuite %s RESULT: %s ********************** \n", obj->name, result);
    TestFW_Report(obj, tmpstr);
    TestFW_GetReport(obj);
    return SuiteResult;
}

uint8_t* TestFW_GetReport(TestFw_t* obj)
{
    QuadFC_Serial_t data;
    data.buffer = (uint8_t*)obj->report;
    data.bufferLength = strlen((char*)obj->report);
    Test_SerialWrite(&data);
    data.buffer[0] = '\0';
    return 0;
}





