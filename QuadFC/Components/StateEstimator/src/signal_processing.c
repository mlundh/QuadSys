/*
 * complementary_filter.c
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
#include "../inc/signal_processing.h"
#include "math.h"
#include "Utilities/inc/my_math.h"
#include "FreeRTOS.h"

#define SIGN(x) ((x > 0) - (x < 0))

void Signal_getEulerAnglesAccel( state_data_t *state, ImuData_t *measurments )
{
  int32_t numerator = 0;
  int32_t denominator = 0;

  // roll calculation
  // tan(roll) = y / (Sign(z) * sqrt(z²+(m*x²)))
  numerator = (int32_t) measurments->imu_data[accl_y];

  denominator = SIGN(measurments->imu_data[accl_z])
      * (int32_t) my_square_root(
          my_square( measurments->imu_data[accl_z], FP_16_16_SHIFT) +
          my_mult( my_square( measurments->imu_data[accl_x], FP_16_16_SHIFT), MY, FP_16_16_SHIFT),
          FP_16_16_SHIFT );
  state->state_bf[roll_bf] = atan2Lerp( numerator, denominator, FP_16_16_SHIFT);
  // Since atan2Lerp has a domain of [0,2pi), and we want a domain of (-pi,pi) we need
  // to remove 2pi if value > pi.
  if(state->state_bf[roll_bf] > BRAD_PI)
  {
    state->state_bf[roll_bf] -= BRAD_2PI;
  }

  // pitch calculation
  // tan(pitch) = -x / sqrt(y² + z²)
  numerator = (int32_t) (-measurments->imu_data[accl_x]);
  denominator = (int32_t) my_square_root( my_square( measurments->imu_data[accl_y], FP_16_16_SHIFT ) +
      my_square( measurments->imu_data[accl_z], FP_16_16_SHIFT ) , FP_16_16_SHIFT);
  state->state_bf[pitch_bf] = atan2Lerp( numerator, denominator, FP_16_16_SHIFT);
  // Since atan2Lerp has a domain of [0,2pi), and we want a domain of (-pi,pi) we need
  // to remove 2pi if value > pi.
  if(state->state_bf[pitch_bf] > BRAD_PI)
  {
    state->state_bf[pitch_bf] -= BRAD_2PI;
  }
  // Indicate which fields are updated and valid.
  state->state_valid_bf |= 1 << roll_bf;
  state->state_valid_bf |= 1 << pitch_bf;
}

void Signal_getEulerAnglesGyro( state_data_t *state, ImuData_t *measurments, control_time_t* time)
{
  state->state_bf[pitch_bf] += my_mult( (int32_t) measurments->imu_data[gyro_y], time->value, TIME_SHIFT_FACTOR );
  state->state_bf[roll_bf]  += my_mult( (int32_t) measurments->imu_data[gyro_x], time->value, TIME_SHIFT_FACTOR );
  state->state_bf[yaw_bf]   += my_mult( (int32_t) measurments->imu_data[gyro_z], time->value, TIME_SHIFT_FACTOR );

  // Indicate which fields are updated and valid.
  state->state_valid_bf |= 1 << pitch_bf;
  state->state_valid_bf |= 1 << roll_bf;
  state->state_valid_bf |= 1 << yaw_bf;
}

void Signal_getRateGyro( state_data_t *state, ImuData_t *measurments )
{
  state->state_bf[pitch_rate_bf] = (int32_t) measurments->imu_data[gyro_y];
  state->state_bf[roll_rate_bf] = (int32_t) measurments->imu_data[gyro_x];
  state->state_bf[yaw_rate_bf] = (int32_t) measurments->imu_data[gyro_z];
  // Indicate which fields are updated and valid.
  state->state_valid_bf |= 1 << pitch_rate_bf;
  state->state_valid_bf |= 1 << roll_rate_bf;
  state->state_valid_bf |= 1 << yaw_rate_bf;
}

void Signal_complemetaryFilter( state_data_t *state_accel, state_data_t *state_gyro, state_data_t *state )
{
  /* Update gyro state to reduce accumulatory errors.*/

  state->state_bf[pitch_bf] = my_mult( (int32_t) state_gyro->state_bf[pitch_bf], (int32_t) FILTER_COEFFICENT_GYRO_FP, FP_16_16_SHIFT ) +
      my_mult( (int32_t) state_accel->state_bf[pitch_bf], (int32_t) FILTER_COEFFICENT_ACCEL_FP, FP_16_16_SHIFT );
  state->state_bf[roll_bf] = my_mult( (int32_t) state_gyro->state_bf[roll_bf], (int32_t) FILTER_COEFFICENT_GYRO_FP, FP_16_16_SHIFT ) +
      my_mult( (int32_t) state_accel->state_bf[roll_bf], (int32_t) FILTER_COEFFICENT_ACCEL_FP, FP_16_16_SHIFT );

  state_gyro->state_bf[pitch_bf] = state->state_bf[pitch_bf];
  state_gyro->state_bf[roll_bf] = state->state_bf[roll_bf];


  // Indicate which fields are updated and valid.
  state->state_valid_bf |= 1 << pitch_bf;
  state->state_valid_bf |= 1 << roll_bf;
}


butterWorth_t* Signal_initButterworth(uint32_t cutOffFrequency, uint32_t samplingFrequency)
{
  butterWorth_t* filter = pvPortMalloc(sizeof(butterWorth_t));
  Signal_calcButterworthCoef(filter, cutOffFrequency, samplingFrequency);
  return filter;
}

uint8_t Signal_calcButterworthCoef(butterWorth_t* obj, uint32_t cutOffFrequency, uint32_t samplingFrequency)
{
  const double ff = (double)cutOffFrequency/(double)samplingFrequency;
  const double ita =1.0/ tan(M_PI*ff);
  const double q=sqrt(2.0);
  double b0 = 1.0 / (1.0 + q*ita + ita*ita);
  double b1= 2*b0;
  double b2= b0;
  double a1 = 2.0 * (ita*ita - 1.0) * b0;
  double a2 = -(1.0 - q*ita + ita*ita) * b0;

  obj->b0 = DOUBLE_TO_FIXED(b0, MAX16f);
  obj->b1 = DOUBLE_TO_FIXED(b1, MAX16f);
  obj->b2 = DOUBLE_TO_FIXED(b2, MAX16f);
  obj->a1 = DOUBLE_TO_FIXED(a1, MAX16f);
  obj->a2 = DOUBLE_TO_FIXED(a2, MAX16f);

  obj->x[0] = 0;
  obj->x[1] = 0;

  obj->y[0] = 0;
  obj->y[1] = 0;

   // y(n)  =  b0*x(n) + b1*x(n-1) + b2*x(n-2) + a1*y(n-1) + a2*y(n-2)
   return 1;
}


int32_t Signal_filterButterworth(butterWorth_t* filter, int32_t input)
{
  // y(n)  =  b0*x(n) + b1*x(n-1) + b2*x(n-2) + a1*y(n-1) + a2*y(n-2)

  // calculate the output
  int32_t y  =   my_mult(filter->b0, input, FP_16_16_SHIFT) + 
                 my_mult(filter->b1, filter->x[0], FP_16_16_SHIFT) + 
                 my_mult(filter->b2, filter->x[1], FP_16_16_SHIFT) + 
                 my_mult(filter->a1, filter->y[0], FP_16_16_SHIFT) + 
                 my_mult(filter->a2, filter->y[1], FP_16_16_SHIFT);
  // update the saved values
  filter->y[1] = filter->y[0];
  filter->y[0] = y;

  filter->x[1] = filter->x[0];
  filter->x[0] = input;
  return y;
}