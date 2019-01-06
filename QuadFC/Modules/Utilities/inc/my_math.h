/*
 * my_math.h
 *
 * Created: 2013-07-08 17:24:21
 *  Author: martin.lundh
 */

#ifndef MY_MATH_H_
#define MY_MATH_H_
#include <stdint.h>

/**
 * Branchless sign function.
 */
#define SIGN(x) ((x > 0) - (x < 0))

/// BRAND stands for binary radian.
#define BRAD_PI_SHIFT (16)
#define BRAD_PI (1 << BRAD_PI_SHIFT)
#define BRAD_HPI (BRAD_PI/2)
#define BRAD_2PI (BRAD_PI*2)


/**
 * @brief Basic lookup+linear interpolation for atan2.
 *
 * Method from:bhttp://www.coranac.com/documents/arctangent/
 *
 * Explanation from same site:
 * The basic framework for the atan2() functions will use the
 * following scheme. First, return early if x and y are both 0. If not,
 * the coordinates will be rotated in till the fall into the first octant
 * and a preliminary angle is constructed as an offset to the real octant.
 *
 * use ATAN_ONE and ATAN_FP for fixed point info of input numbers.
 *
 * @param y numerator, fixed point with scale shift_factor.
 * @param x denominator, fixed point with scale shift_factor.
 * @return [0,2pi), where pi ~ 0x4000.
 */
uint32_t atan2Lerp(int32_t y, int32_t x, uint32_t shift_factor);


uint32_t my_square_root( uint32_t a_nInput, uint32_t shift);
uint32_t my_square( int32_t val, uint32_t shift);
int32_t my_div( int32_t numerator, int32_t denominator, uint32_t shift_factor);
int32_t my_mult( int32_t operand1, int32_t operand2, int32_t shift_factor);
int32_t my_add( int32_t operand1, int32_t operand2);
int32_t my_sub( int32_t operand1, int32_t operand2);

#endif /* MY_MATH_H_ */
