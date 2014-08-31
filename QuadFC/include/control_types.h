/*
 * control_types.h
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
#ifndef CONTROL_TYPES_H_
#define CONTROL_TYPES_H_

#include "compiler.h"



/* A block time of 0 ticks simply means "don't block". */
//#define mainDONT_BLOCK                          (0)

/*---------------------------------------------------------*/
/*
 * Struct containing the state information.
 */
typedef struct state_data
{
  int32_t pitch;
  int32_t roll;
  int32_t yaw;
  int32_t pitch_rate;
  int32_t roll_rate;
  int32_t yaw_rate;
  int32_t x_pos;
  int32_t y_pos;
  int32_t z_pos;
  int32_t x_vel;
  int32_t y_vel;
  int32_t z_vel;
} state_data_t;

/*---------------------------------------------------------*/
/*
 * Struct containing the control parameters.
 */
typedef struct control_values_pid
{
  int32_t roll_p;
  int32_t roll_i;
  int32_t roll_d;
  int32_t pitch_p;
  int32_t pitch_i;
  int32_t pitch_d;
  int32_t yaw_p;
  int32_t yaw_i;
  int32_t yaw_d;
  int32_t altitude_p;
  int32_t altitude_i;
  int32_t altitude_d;
} control_values_pid_t;

/*---------------------------------------------------------*/
/*
 * Struct containing the control signal.
 */
typedef struct control_signal
{
  int32_t thrust;
  int32_t torque_yaw;
  int32_t torque_pitch;
  int32_t torque_roll;
} control_signal_t;

/*QuadFC states. The Quadcopter can only be in one of the following staes*/
typedef enum quadfc_state
{
  fc_disarmed = 1,
  fc_armed_rate_mode = 2,
  fc_armed_angle_mode = 3,
  fc_config_error = 4,
  fc_initializing = 5,
  fc_configure = 6,
  fc_arming = 7,
} quadfc_state_t;

typedef enum available_log_param1
{
  time_ =             1 << 0,
  pitch_ =            1 << 1,
  roll_ =             1 << 2,
  yaw_ =              1 << 3,
  pitch_rate_ =       1 << 4,
  roll_rate_ =        1 << 5,
  yaw_rate_ =         1 << 6,
  x_pos_ =            1 << 7,
  y_pos_ =            1 << 8,
  z_pos_ =            1 << 9,
  x_vel_ =            1 << 10,
  y_vel_ =            1 << 11,
  z_vel_ =            1 << 12,
  ctrl_torque_pitch = 1 << 13,
  ctrl_torque_roll =  1 << 14,
  ctrl_torque_yaw =   1 << 15,
  ctrl_thrust =       1 << 16,
//ctrl_roll_rate =    1 << 17,
//ctrl_yaw_rate =     1 << 18,
//ctrl_x_pos =        1 << 19,
//ctrl_y_pos =        1 << 20,
//ctrl_z_pos =        1 << 21,
//ctrl_x_vel =        1 << 22,
//ctrl_y_vel =        1 << 23,
//ctrl_z_vel =        1 << 24,
} available_log_param1_t;

typedef enum available_log_param2
{
  ctrl_error_pitch =        1 << 0,
  ctrl_error_roll =         1 << 1,
  ctrl_error_yaw =          1 << 2,
  ctrl_error_pitch_rate =   1 << 3,
  ctrl_error_roll_rate =    1 << 4,
  ctrl_error_yaw_rate =     1 << 5,
  ctrl_error_x_pos =        1 << 6,
  ctrl_error_y_pos =        1 << 7,
  ctrl_error_z_pos =        1 << 8,
  ctrl_error_x_vel =        1 << 9,
  ctrl_error_y_vel =        1 << 10,
  ctrl_error_z_vel =        1 << 11,
  setpoint_pitch =          1 << 12,
  setpoint_roll =           1 << 13,
  setpoint_yaw =            1 << 14,
  setpoint_pitch_rate =     1 << 15,
  setpoint_roll_rate =      1 << 16,
  setpoint_yaw_rate =       1 << 17,
  setpoint_x_pos =          1 << 18,
  setpoint_y_pos =          1 << 19,
  setpoint_z_pos =          1 << 20,
  setpoint_x_vel =          1 << 21,
  setpoint_y_vel =          1 << 22,
  setpoint_z_vel =          1 << 23,
} available_log_param2_t;
#endif /* CONTROL_TYPES_H_ */
