/*
 * my_math.c
 *
 * Copyright (C) 2014  Martin Lundh
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

#include "../inc/my_math.h"
#include "HMI/inc/led_control_task.h"

/*atan2() domain: -pi to pi*/
int32_t my_atan2( int32_t numerator, int32_t denominator )
{

  if ( denominator > 0 )
  {
    return my_atan( my_div( numerator, denominator ) );
  }
  else if ( numerator >= 0 && denominator < 0 )
  {
    return (my_atan( my_div( numerator, denominator ) )); //+ MY_PI);
  }
  else if ( numerator < 0 && denominator < 0 )
  {
    return (my_atan( my_div( numerator, denominator ) )); // - MY_PI);
  }
  else if ( numerator > 0 && denominator == 0 )
  {
    return (MY_PI_HALF);
  }
  else if ( numerator < 0 && denominator == 0 )
  {
    return (-MY_PI_HALF);
  }
  else if ( numerator == 0 && denominator == 0 )
  {
    return (0);
  }
  return 0;
}
/*my_tan() uses a lookup table of arctan. The lookup table is constructed so that the
 input (index) has a fixed resolution. This means that the table is least accurate
 around 0 deg. This is a consequence to the way data is found in the table. It is
 not ideal, but good enough for now.

 Lookup table domain [65deg to -65deg]

 arg should be shifted by SHIFT_EXP.

 */

int32_t my_atan( int32_t arg )
{
  uint16_t index = ((LOOKUP_ATAN_LENGTH / 2) + ((arg) / (LOOKUP_ATAN_RESOLUTION)) - 1);
  if ( index < 0 || index > LOOKUP_ATAN_LENGTH )
  {
    return 0xF0000000;
  }
  return (int32_t) atan_lookup[index];
}

// Method of calculating square root as described in 
// http://en.wikipedia.org/wiki/Methods_of_computing_square_roots under the heading
// "Binary numeral system (base 2)"
uint32_t my_square_root( uint32_t a_nInput )
{
  uint32_t op = a_nInput << SHIFT_EXP;
  uint32_t res = 0;
  uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type

  // "one" starts at the highest power of four <= than the argument.
  while ( one > op )
  {
    one >>= 2;
  }

  while ( one != 0 )
  {
    if ( op >= res + one )
    {
      op = op - (res + one);
      res = res + 2 * one;
    }
    res >>= 1;
    one >>= 2;
  }
  return res;
}

uint32_t my_square( int16_t val )
{
  return (my_mult( val, val ));
}

int32_t my_sgn( int16_t val )
{
  if ( val < 0 )
  {
    return -1;
  }
  else if ( val == 0 )
  {
    return 0;
  }
  else
  {
    return 1;
  }
  return 0;
}

int32_t my_div( int32_t numerator, int32_t denominator )
{
  if ( denominator == 0 )
  {
    // ERROR!
  }
  return ((numerator << SHIFT_EXP) / denominator);
}

int32_t my_mult( int32_t operand1, int32_t operand2 )
{
  /*The result of a multiplication will always fit in 2*n bits,
   where n is the number of bits in the larger of the two operands. Integers of 32
   bits should be cast to 64 bits, and smaller integers can be cast to 32 bits.*/

  int64_t temp = ((int64_t) operand1 * (int64_t) operand2);
  /*For signed integers, an overflow has not occurred if the high-order bits and
   the sign bit on the lower half of the product are all zeros or ones. This means
   that for a 64-bit product the upper 33 bits would need to be all
   zeros or ones. In this application a shift factor is used for fixed point
   arithmetics, and it is therefore OK to use more than the upper 33 bits.*/
  if ( (temp >> (31 - SHIFT_EXP)) != (temp >> (32 - SHIFT_EXP)) )
  {
    //ERROR!
    Led_Set(led_error_int_overflow);
  }

  return (int32_t) (temp >> SHIFT_EXP);
}

int32_t my_mult2( int32_t operand1, int32_t operand2, int32_t shift_factor)
{
  /*The result of a multiplication will always fit in 2*n bits,
   where n is the number of bits in the larger of the two operands. Integers of 32
   bits should be cast to 64 bits, and smaller integers can be cast to 32 bits.*/

  int64_t temp = ((int64_t) operand1 * (int64_t) operand2);
  /*For signed integers, an overflow has not occurred if the high-order bits and
   the sign bit on the lower half of the product are all zeros or ones. This means
   that for a 64-bit product the upper 33 bits would need to be all
   zeros or ones. In this application a shift factor is used for fixed point
   arithmetics, and it is therefore OK to use more than the upper 33 bits.*/
  if ( (temp >> (31 - shift_factor)) != (temp >> (32 - shift_factor)) )
  {
    //ERROR!
    Led_Set(led_error_int_overflow);
  }

  return (int32_t) (temp >> shift_factor);
}
