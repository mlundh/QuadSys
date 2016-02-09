/*
 * common_types.h
 *
 * Copyright (C) 2014 martin
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
#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_
#include "stdint.h"

typedef struct SpObj SpObj_t;
typedef struct StateHandler StateHandler_t;

typedef enum state_names
{
  pitch = 0,
  roll = 1,
  yaw = 2,
  pitch_rate = 3,
  roll_rate = 4,
  yaw_rate = 5,
  x_pos = 6,
  y_pos = 7,
  z_pos = 8,
  x_vel = 9,
  y_vel = 10,
  z_vel = 11,
  nr_states = 12,
}state_names_t;

typedef struct state_data
{
  int32_t state_vector[nr_states];
  uint8_t confidence[nr_states];
}state_data_t;

typedef enum control_signal_names
{
  u_pitch = 0,
  u_roll = 1,
  u_yaw = 2,
  u_thrust = 3,
}control_signal_names_t;

typedef struct control_signal
{
  int32_t control_signal[4];
}control_signal_t;

typedef enum ImuNames
{
  accl_x = 0,
  accl_y = 1,
  accl_z = 2,
  gyro_x = 3,
  gyro_y = 4,
  gyro_z = 5,
  mag_x = 6,
  mag_y = 7,
  mag_z = 8,
  temp_imu = 9,
  nr_imu_values = 10,
}ImuNames_t;

typedef struct ImuData
{
  int16_t imu_data[nr_imu_values];
  uint8_t confidence[nr_imu_values];
}ImuData_t;

typedef struct ImuOrientation
{
  int8_t x_sign;
  int8_t y_sign;
  int8_t z_sign;
}ImuOrientation_t;

typedef struct Imu
{
  ImuData_t ImuOffset;
  ImuData_t ImuData;
  ImuData_t tempData;
  ImuOrientation_t Orient;
  void *internals;
}Imu_t;



#endif /* COMMON_TYPES_H_ */
