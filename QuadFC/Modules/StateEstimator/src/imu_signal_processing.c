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
#include "StateEstimator/inc/imu_signal_processing.h"
#include "Utilities/inc/my_math.h"

void get_euler_angles_accel( state_data_t *state, ImuData_t *measurments )
{
  // TODO DO NOT USE UNTILL FIXED. MY_ATAN DOES NOT WORK NOW!
  int32_t numerator = 0;
  int32_t denominator = 0;

  // pitch calculation
  numerator = (int32_t) (measurments->imu_data[accl_y]);
  denominator = (my_sgn( measurments->imu_data[accl_z] ))
      * (int32_t) (my_square_root( (my_square( measurments->imu_data[accl_z] )
      + my_mult( my_square( measurments->imu_data[accl_x] ), MY )) ));

  state->state_vector[pitch] = my_atan2( numerator, denominator );
  // roll calculation
  numerator = (int32_t) (measurments->imu_data[accl_x]);
  denominator = (int32_t) (my_square_root( my_square( measurments->imu_data[accl_x] )
      + my_square( measurments->imu_data[accl_z] ) ));

  state->state_vector[roll] = my_atan2( numerator, denominator );
}

void get_euler_angles_gyro( state_data_t *state, ImuData_t *measurments )
{
  /*
   * TODO
   * DO NOT USE UNTIL FIXED
   */
  state->state_vector[pitch] += my_mult( ((int32_t) measurments->imu_data[gyro_x] *
      (int32_t) CONVERT_IMU_TO_RECEIVER_UNITS), TIME_STEP );
  state->state_vector[roll] += my_mult( ((int32_t) measurments->imu_data[gyro_y] *
      (int32_t) CONVERT_IMU_TO_RECEIVER_UNITS), TIME_STEP );
  state->state_vector[yaw] += my_mult( ((int32_t) measurments->imu_data[gyro_z] *
      (int32_t) CONVERT_IMU_TO_RECEIVER_UNITS), TIME_STEP );
}

void get_rate_gyro( state_data_t *state, ImuData_t *measurments )
{
  state->state_vector[pitch_rate] = ((int32_t) measurments->imu_data[gyro_x]) >> 6;
  state->confidence[pitch_rate] = 1;

  state->state_vector[roll_rate] = ((int32_t) measurments->imu_data[gyro_y]) >> 6;
  state->confidence[roll_rate] = 1;

  state->state_vector[yaw_rate] = ((int32_t) measurments->imu_data[gyro_z]) >> 6;
  state->confidence[roll_rate] = 1;
}

void complemetary_filter( state_data_t *state_accel, state_data_t *state_gyro, state_data_t *state )
{
  state->state_vector[pitch] = my_mult( (int32_t) state_gyro->state_vector[pitch],
      (int32_t) FILTER_COEFFICENT_GYRO ) + my_mult( (int32_t) state_accel->state_vector[pitch],
      (int32_t) FILTER_COEFFICENT_ACCEL );
  state->state_vector[roll] = my_mult( (int32_t) state_gyro->state_vector[roll],
      (int32_t) FILTER_COEFFICENT_GYRO ) + my_mult( (int32_t) state_accel->state_vector[roll],
      (int32_t) FILTER_COEFFICENT_ACCEL );
  /*Update gyro state to reduce accumulatory errors.*/
  state_gyro->state_vector[pitch] = state->state_vector[pitch];
  state_gyro->state_vector[roll] = state->state_vector[roll];
}

/*Translates the received signal into fixed point used in project*/
void translate_receiver_signal_rate( state_data_t *setpoint, receiver_data_t *received_data )
{
  setpoint->state_vector[z_vel] = (received_data->ch0);
  setpoint->state_vector[roll_rate] = -((received_data->ch1 - SATELLITE_CH_CENTER) );
  setpoint->state_vector[pitch_rate] = -((received_data->ch2 - SATELLITE_CH_CENTER));
  setpoint->state_vector[yaw_rate] = -((received_data->ch3 - SATELLITE_CH_CENTER) );

}
