/*
 * my_math.h
 *
 * Created: 2013-07-08 17:24:21
 *  Author: martin.lundh
 */

#ifndef MY_MATH_H_
#define MY_MATH_H_
#include "compiler.h"

#define MY_PI 31416 // 3,1415 * MULT_FACTOR
#define MY_PI_HALF 15708 // 3,1415/2 * MULT_FACTOR
#define LOOKUP_ATAN_RESOLUTION 170 // 0,017 * MULT_FACTOR
#define LOOKUP_ATAN_LENGTH 255

static const int16_t atan_lookup[LOOKUP_ATAN_LENGTH] = {
    -73, -72, -72, -72, -72, -72, -71, -71, -71, -71, -71, -70, -70, -70, -70, -69
    , -69, -69, -69, -68, -68, -68, -68, -67, -67, -67, -67, -66, -66, -66, -65, -65
    , -65, -65, -64, -64, -64, -63, -63, -63, -62, -62, -62, -61, -61, -61, -60, -60
    , -59, -59, -59, -58, -58, -57, -57, -56, -56, -56, -55, -55, -54, -54, -53, -53
    , -52, -52, -51, -51, -50, -50, -49, -48, -48, -47, -47, -46, -45, -45, -44, -43
    , -43, -42, -41, -41, -40, -39, -39, -38, -37, -36, -36, -35, -34, -33, -32, -31
    , -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16
    , -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6,
    7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 19, 20, 21, 21, 22, 23, 24, 25, 26, 27, 28, 29
    , 30, 31, 31, 32, 33, 34, 35, 36, 36, 37, 38, 39, 39, 40, 41, 41, 42, 43, 43, 44, 45, 45
    , 46, 47, 47, 48, 48, 49, 50, 50, 51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 56, 57
    , 57, 58, 58, 59, 59, 59, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 65, 65
    , 65, 66, 66, 66, 66, 67, 67, 67, 67, 68, 68, 68, 68, 69, 69, 69, 69, 70, 70, 70, 70, 71
    , 71, 71, 71, 71, 72, 72, 72, 72, 72, 73, 73
};

int32_t my_atan2( int32_t numerator, int32_t denominator );
int32_t my_atan( int32_t arg );
uint32_t my_square_root( uint32_t a_nInput );
uint32_t my_square( int16_t val );
int32_t my_sgn( int16_t val );
int32_t my_div( int32_t numerator, int32_t denominator );
int32_t my_mult( int32_t operand1, int32_t operand2 );
#endif /* MY_MATH_H_ */
