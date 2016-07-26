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




#define ATAN_LUT_FP (17)
#define LUT_TO_BRAD_SCALE_SHIFT (ATAN_LUT_FP - BRAD_PI_SHIFT)

//#define ATAN_ONE (0x1000)
//#define ATAN_FP (12)
// Some constants for dealing with atanLUT.
#define ATAN_NR_ENTRIES_FACTOR (7) // 128 Entries in LUT
//static const uint32_t ATANLUT_STRIDE = ATAN_ONE / ATAN_NR_ENTRIES;
//#define ATANLUT_STRIDE_SHIFT (5) // difference ATAN_FP - ATAN_NR_ENTRIES;

/**
 * @brief Get the octant a coordinate pair is in.
 *
 * Method from: http://www.coranac.com/documents/arctangent/
 *
 * Explanation from same site:
 * If necessary, rotate the point by π to bring it into the top hemisphere,
 * then rotate by ½π to get to the first quadrant. We need to keep track of
 * the performed rotations in the form of φi.
 * Once inside the first quadrant, we continue to rotate: if x>y, rotate
 * by −¼π. Now, if you look at the equations, you might think this is a bad
 * idea thanks to the division by √2. However, this scaling doesn't matter,
 * because the coordinate division will remove any trace of it anyway.
 */
#define OCTANTIFY(_x, _y, _o)   do {                            \
    int _t; _o= 0;                                              \
    if(_y<  0)  {            _x= -_x;   _y= -_y; _o += 4; }     \
    if(_x<= 0)  { _t= _x;    _x=  _y;   _y= -_t; _o += 2; }     \
    if(_x<=_y)  { _t= _y-_x; _x= _x+_y; _y=  _t; _o += 1; }     \
} while(0);
//- See more at: http://www.coranac.com/documents/arctangent/


/**
 * @brief Arctangens LUT. Interval: [0, 1] (one=128); PI=0x20000
 *
 * Method from: http://www.coranac.com/documents/arctangent/
 *
 * Explanation from same site: *
 * This particular LUT needs to have values for t = y/x∈[0, 1]
 * (i.e, φ∈[0, ¼π]) Note that 1.0 is included in this range, as
 * this is required for linear interpolation. The
 * entry for 1.0 is at position 128, so that the indices are
 * effectively Q7 numbers. For accuracy, I'll scale each entry
 * so that as far as the LUT is concerned, ¼π ≡ 0x8000. This
 * will be shifted down at the end of the calculations.
 * PI=0x20000
 */
const unsigned short atanLUT[130] =
{
    0x0000,0x0146,0x028C,0x03D2,0x0517,0x065D,0x07A2,0x08E7,
    0x0A2C,0x0B71,0x0CB5,0x0DF9,0x0F3C,0x107F,0x11C1,0x1303,
    0x1444,0x1585,0x16C5,0x1804,0x1943,0x1A80,0x1BBD,0x1CFA,
    0x1E35,0x1F6F,0x20A9,0x21E1,0x2319,0x2450,0x2585,0x26BA,
    0x27ED,0x291F,0x2A50,0x2B80,0x2CAF,0x2DDC,0x2F08,0x3033,
    0x315D,0x3285,0x33AC,0x34D2,0x35F6,0x3719,0x383A,0x395A,
    0x3A78,0x3B95,0x3CB1,0x3DCB,0x3EE4,0x3FFB,0x4110,0x4224,
    0x4336,0x4447,0x4556,0x4664,0x4770,0x487A,0x4983,0x4A8B,
    // 64
    0x4B90,0x4C94,0x4D96,0x4E97,0x4F96,0x5093,0x518F,0x5289,
    0x5382,0x5478,0x556E,0x5661,0x5753,0x5843,0x5932,0x5A1E,
    0x5B0A,0x5BF3,0x5CDB,0x5DC1,0x5EA6,0x5F89,0x606A,0x614A,
    0x6228,0x6305,0x63E0,0x64B9,0x6591,0x6667,0x673B,0x680E,
    0x68E0,0x69B0,0x6A7E,0x6B4B,0x6C16,0x6CDF,0x6DA8,0x6E6E,
    0x6F33,0x6FF7,0x70B9,0x717A,0x7239,0x72F6,0x73B3,0x746D,
    0x7527,0x75DF,0x7695,0x774A,0x77FE,0x78B0,0x7961,0x7A10,
    0x7ABF,0x7B6B,0x7C17,0x7CC1,0x7D6A,0x7E11,0x7EB7,0x7F5C,
    // 128
    0x8000,0x80A2
};

// Basic lookup+linear interpolation for atan2.
// Returns [0,2pi), where pi ~ 0x4000.
uint32_t atan2Lerp(int32_t y, int32_t x, uint32_t shift_factor)
{
  if(y==0)
  {
    return ( x>= 0 ? 0 : BRAD_PI);
  }

  int phi;               //!< preliminary angle constructed from the octant offset.
  uint32_t t, fa, fb, h; //!< adjustment angle and variables used for interpolation.

  OCTANTIFY(x, y, phi);
  phi *= BRAD_PI/4;

  // Since we multiply with a shifted value we have to shift back afterwards.
  uint32_t aTanLut_strideShift = shift_factor - ATAN_NR_ENTRIES_FACTOR;

  // Stride describes the "width" of each lut entry in relation to the input scale.
  uint32_t aTanLut_stride = 1 << aTanLut_strideShift;

  t= my_div(y, x, shift_factor);
  h= t % aTanLut_stride;
  fa= atanLUT[t/aTanLut_stride  ];
  fb= atanLUT[t/aTanLut_stride+1];

  return (phi + (( fa + ((fb-fa)*h >> aTanLut_strideShift) ) >> LUT_TO_BRAD_SCALE_SHIFT));
}




// Method of calculating square root as described in 
// http://en.wikipedia.org/wiki/Methods_of_computing_square_roots under the heading
// "Binary numeral system (base 2)"
uint32_t my_square_root( uint32_t a_nInput, uint32_t shift)
{
  // used to shift up by shift since we want the answer to be
  // fixed point too. This limited resolution to 16 bits (<1 in 16.16 FP).
  // If we instead wait and shift after the calculation we loose resolution
  // instead of range. We loose 8 bits of resolution. This is acceptable.
  uint32_t op = a_nInput;
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

  // by waiting to shift until after the calc we get the
  // result of the full 32 bits (16 integer bits) but we
  // loose 8 bit resolution.
  res = res << 8;
  return res;
}

uint32_t my_square( int32_t val, uint32_t shift)
{
  return (my_mult( val, val, shift));
}

int32_t my_div( int32_t numerator, int32_t denominator, uint32_t shift_factor)
{
  if ( denominator == 0 )
  {
    // ERROR
    return 0xFFFFFFFF;
  }
  return (int32_t)(((int64_t)numerator << shift_factor) / denominator);
}


int32_t my_mult( int32_t operand1, int32_t operand2, int32_t shift_factor)
{
  /*The result of a multiplication will always fit in 2*n bits,
   where n is the number of bits in the larger of the two operands. Integers of 32
   bits should be cast to 64 bits, and smaller integers can be cast to 32 bits.*/

  int64_t temp = ((int64_t) operand1 * (int64_t) operand2) >> shift_factor;
  /*For signed integers, an overflow has not occurred if the high-order bits and
   the sign bit on the lower half of the product are all zeros or ones. This means
   that for a 64-bit product the upper 33 bits would need to be all
   zeros or ones.*/
  if ( (temp >> (31)) != (temp >> (32)) )
  {
    //TODO error report!
    (void) operand2;
  }

  return (int32_t) (temp);
}

int32_t my_add( int32_t operand1, int32_t operand2)
{
  /*The result of an addition will always fit in 2*n bits,
   where n is the number of bits in the larger of the two operands. Integers of 32
   bits should be cast to 64 bits, and smaller integers can be cast to 32 bits.*/

  int64_t temp = ((int64_t) operand1 + (int64_t) operand2);
  /*For signed integers, an overflow has not occurred if the high-order bits and
   the sign bit on the lower half of the product are all zeros or ones. This means
   that for a 64-bit product the upper 33 bits would need to be all
   zeros or ones.*/
  if ( (temp >> (31)) != (temp >> (32)) )
  {
    //TODO error report!
  }

  return (int32_t) (temp);
}

int32_t my_sub( int32_t operand1, int32_t operand2)
{
  /*The result of an addition will always fit in 2*n bits,
   where n is the number of bits in the larger of the two operands. Integers of 32
   bits should be cast to 64 bits, and smaller integers can be cast to 32 bits.*/

  int64_t temp = ((int64_t) operand1 - (int64_t) operand2);
  /*For signed integers, an overflow has not occurred if the high-order bits and
   the sign bit on the lower half of the product are all zeros or ones. This means
   that for a 64-bit product the upper 33 bits would need to be all
   zeros or ones.*/
  if ( (temp >> (31)) != (temp >> (32)) )
  {
    //TODO error report!
  }

  return (int32_t) (temp);
}


