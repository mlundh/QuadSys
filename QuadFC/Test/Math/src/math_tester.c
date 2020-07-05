/*
 * MathTester.c
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

#include "Test/Math/math_tester.h"
#include "Utilities/inc/my_math.h"
#include <math.h>
#include <stdio.h>

#include "HAL/QuadFC/QuadFC_Gpio.h"


#define TMP_STRING_LENGTH (100)

#define MAX16f (65536.0)
//#define MAX14f (16384.0)

#define INT_TO_FIXED(x,y)         ((x) << (y))
#define DOUBLE_TO_FIXED(x, y)     ((int32_t)(x * y + 0.5))
#define FIXED_TO_INT(x,y)         ((x) >> (y))
#define FIXED_TO_DOUBLE(x, y)     (((double)x) / (y))

void MathTest_GetTCs(TestFw_t* obj)
{
  TestFW_RegisterTest(obj, "Tan2", MathTest_TestTan2);
  TestFW_RegisterTest(obj, "sqrt", MathTest_TestSqrt);
  TestFW_RegisterTest(obj, "sqrtPerf", MathTest_TestSqrtPerformance);

}

uint8_t MathTest_TestTan2(TestFw_t* obj)
{
  //Generate input data.
  // degrees:  10   20    30     40    50    60     70   80
  double x[]={0.98, 0.94, 0.87, 0.77, 0.64, 0.50, 0.34, 0.17};
  double y[]={0.17, 0.34, 0.50, 0.64, 0.77, 0.87, 0.94, 0.98};

  char tmpstr[TMP_STRING_LENGTH] = {0};

  double signx[] = {1,-1,-1,1};
  double signy[] = {1,1,-1,-1};

  uint8_t result = 1;
  for(int j = 0; j < 4; j++)
  {
    for(int i = 0; i < 8; i++)
    {
      double x_i = x[i]*signx[j];
      double y_i = y[i]*signy[j];
      //Calculate reference value
      double result_ref = atan2 (y_i,x_i);
      // domain of reference is [-pi, pi], we have to move to [0,2pi).
      if(result_ref < 0)
      {
        result_ref = (2.0*M_PI) + result_ref;
      }

      //Use DUT (my_math) to calculate based on the same values.
      int32_t result_dut = atan2Lerp ( DOUBLE_TO_FIXED(y_i,MAX16f),DOUBLE_TO_FIXED(x_i,MAX16f),16);
      double resDutFloat = FIXED_TO_DOUBLE(result_dut, MAX16f) * 3.1415; // Times pi due to scaling from atan2Lerp

      //Calculate absolute difference.
      double difference = result_ref - resDutFloat;
      difference *= SIGN(difference);
      if(difference > 0.001) // 0.001 rad = 0,057 deg
      {
        snprintf (tmpstr, TMP_STRING_LENGTH,
            "Ref value and dut value differs to much for input values: %d, %d.\n", (int)x_i, (int)y_i);
        TestFW_Report(obj, tmpstr);
        result = 0;
      }
    }
  }
  return result;
}

uint8_t MathTest_TestSqrt(TestFw_t* obj)
{
  //Generate input data.
  double testVector[]={0.17, 0.34, 0.50, 0.64, 0.77, 0.87, 0.94, 0.98, 5000, 9000};

  int testVectorLength = sizeof(testVector)/sizeof(testVector[0]);
  char tmpstr[TMP_STRING_LENGTH] = {0};


  uint8_t result = 1;
  for(int i = 0; i < testVectorLength; i++)
  {
    //Calculate reference value
    double result_ref = sqrt (testVector[i]);

    //Use DUT (my_math) to calculate based on the same values.
    int32_t result_dut = my_square_root ( DOUBLE_TO_FIXED(testVector[i],MAX16f), 16);
    double resDutFloat = FIXED_TO_DOUBLE(result_dut, MAX16f);

    //Calculate absolut difference.
    double difference = result_ref - resDutFloat;
    difference *= SIGN(difference);
    if(difference > 0.01)
    {
      snprintf (tmpstr, TMP_STRING_LENGTH,
          "Ref value and dut value differs by: %d, which is much. Input value: %d.\n", (int)difference, (int)testVector[i]);
      TestFW_Report(obj, tmpstr);
      result = 0;
    }
  }
  return result;
}

uint8_t MathTest_TestSqrtPerformance(TestFw_t* obj)
{
  //Generate input data.
  double testVector[]={0.17, 0.34, 0.50, 0.64, 0.77, 0.87, 0.94, 0.98, 5000, 9000};

  int testVectorLength = sizeof(testVector)/sizeof(testVector[0]);

  uint32_t testVectorInt[testVectorLength];
  for(int k = 0; k < testVectorLength; k++)
  {
    testVectorInt[k] = DOUBLE_TO_FIXED(testVector[k],MAX16f);
  }
    //Calculate reference value
    double result_ref;
    Gpio_TogglePin( ledmode );
    for(int j = 0; j < 100000; j++)
    {
      result_ref = sqrt (testVector[j%testVectorLength]);
      (void)result_ref;
    }
    Gpio_TogglePin( ledmode );
    Gpio_TogglePin( ledStatus );
    int32_t result_dut;
    for(int j = 0; j < 100000; j++)
    {
      result_dut =  my_square_root ( testVectorInt[j%testVectorLength], 16);
      (void)result_dut;
    }
    Gpio_TogglePin( ledStatus );


  return 1;
}
