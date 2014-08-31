/*
 * control_algorithm.c
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
#include "control_algorithm.h"
#include "my_math.h"
#include "globals.h"

void limit_value( int32_t *val, int32_t limit );

#define CTRL_P_LIMIT 1000
#define CTRL_I_LIMIT 9000
#define CTRL_I_LIMIT_YAW 15000
#define CTRL_D_LIMIT 300

void calc_control_signal_angle_pid( int32_t motor_setpoint[], uint32_t nr_motors, control_values_pid_t *parameters_pid, control_values_pid_t *ctrl_error, state_data_t *state,
    state_data_t *setpoint, control_signal_t *ctrl_signal )
{

  static control_signal_t error_d_last;

  /*Update control error - Error is expressed in receiver resolution.*/
  ctrl_error->pitch_p = (setpoint->pitch - state->pitch);
  ctrl_error->pitch_i += (setpoint->pitch - state->pitch);
  ctrl_error->pitch_d = ((setpoint->pitch - state->pitch) - error_d_last.torque_pitch);

  ctrl_error->roll_p = (setpoint->roll - state->roll);
  ctrl_error->roll_i += (setpoint->roll - state->roll);
  ctrl_error->roll_d = ((setpoint->roll - state->roll) - error_d_last.torque_roll);

  ctrl_error->yaw_p = (setpoint->yaw_rate - state->yaw_rate);
  ctrl_error->yaw_i += (setpoint->yaw_rate - state->yaw_rate);
  ctrl_error->yaw_d = ((setpoint->yaw_rate - state->yaw_rate) - error_d_last.torque_yaw);
  // TODO no altitude control yet.

  limit_value( &ctrl_error->pitch_p, CTRL_P_LIMIT );
  limit_value( &ctrl_error->roll_p, CTRL_P_LIMIT );
  limit_value( &ctrl_error->yaw_p, CTRL_P_LIMIT );

  limit_value( &ctrl_error->pitch_i, CTRL_I_LIMIT );
  limit_value( &ctrl_error->roll_i, CTRL_I_LIMIT );
  limit_value( &ctrl_error->yaw_i, CTRL_I_LIMIT );

  limit_value( &ctrl_error->pitch_d, CTRL_D_LIMIT );
  limit_value( &ctrl_error->roll_d, CTRL_D_LIMIT );
  limit_value( &ctrl_error->yaw_d, CTRL_D_LIMIT );

  error_d_last.torque_pitch = ctrl_error->pitch_p;
  error_d_last.torque_roll = ctrl_error->roll_p;
  error_d_last.torque_yaw = ctrl_error->pitch_p;

  /*Calculate control signal PID*/
  ctrl_signal->torque_pitch = (my_mult( parameters_pid->pitch_p, ctrl_error->pitch_p ) + my_mult( parameters_pid->pitch_i, ctrl_error->pitch_i )
      + my_mult( parameters_pid->pitch_d, ctrl_error->pitch_d ));
  ctrl_signal->torque_roll = (my_mult( parameters_pid->roll_p, ctrl_error->roll_p ) + my_mult( parameters_pid->roll_i, ctrl_error->roll_i )
      + my_mult( parameters_pid->roll_d, ctrl_error->roll_d ));
  ctrl_signal->torque_yaw = (my_mult( parameters_pid->yaw_p, ctrl_error->yaw_p ) + my_mult( parameters_pid->yaw_i, ctrl_error->yaw_i )
      + my_mult( parameters_pid->yaw_d, ctrl_error->yaw_d ));
  ctrl_signal->thrust = my_mult( parameters_pid->altitude_p, setpoint->z_vel ); // No automatic altitude control yet. TODO

  /*Make sure that the motors are always on when in one of the flight modes*/
  if ( ctrl_signal->thrust < 50 )
  {
    ctrl_signal->thrust = 50;
  }
  if ( nr_motors == 4 )
  {
    control_allocation_quad_x( ctrl_signal, motor_setpoint );
  }

}

void calc_control_signal_rate_pid( int32_t motor_setpoint[], uint32_t nr_motors, control_values_pid_t *parameters_pid, control_values_pid_t *ctrl_error, state_data_t *state,
    state_data_t *setpoint, control_signal_t *ctrl_signal )
{

  static control_signal_t error_d_last = { };

  /*Update control error - Remember that the state and setpoint uses MULT_FACTOR*/
  ctrl_error->pitch_p = (setpoint->pitch_rate - state->pitch_rate);
  ctrl_error->pitch_i += (setpoint->pitch_rate - state->pitch_rate);
  ctrl_error->pitch_d = ((setpoint->pitch_rate - state->pitch_rate) - error_d_last.torque_pitch);
  ctrl_error->roll_p = (setpoint->roll_rate - state->roll_rate);
  ctrl_error->roll_i += (setpoint->roll_rate - state->roll_rate);
  ctrl_error->roll_d = ((setpoint->roll_rate - state->roll_rate) - error_d_last.torque_roll);
  ctrl_error->yaw_p = (setpoint->yaw_rate - state->yaw_rate);
  ctrl_error->yaw_i += (setpoint->yaw_rate - state->yaw_rate);
  ctrl_error->yaw_d = ((setpoint->yaw_rate - state->yaw_rate) - error_d_last.torque_yaw);
  // TODO no altitude control yet.

  error_d_last.torque_pitch = ctrl_error->pitch_p;
  error_d_last.torque_roll = ctrl_error->roll_p;
  error_d_last.torque_yaw = ctrl_error->yaw_p;

  /*Calculate control signal*/
  ctrl_signal->torque_pitch = (my_mult( parameters_pid->pitch_p, ctrl_error->pitch_p ) + my_mult( parameters_pid->pitch_i, ctrl_error->pitch_i )
      + my_mult( parameters_pid->pitch_d, ctrl_error->pitch_d )) >> SHIFT_EXP;
  ctrl_signal->torque_roll = (my_mult( parameters_pid->roll_p, ctrl_error->roll_p ) + my_mult( parameters_pid->roll_i, ctrl_error->roll_i )
      + my_mult( parameters_pid->roll_d, ctrl_error->roll_d )) >> SHIFT_EXP;
  ctrl_signal->torque_yaw = (my_mult( parameters_pid->yaw_p, ctrl_error->yaw_p ) + my_mult( parameters_pid->yaw_i, ctrl_error->yaw_i )
      + my_mult( parameters_pid->yaw_d, ctrl_error->yaw_d )) >> SHIFT_EXP;
  // TODO No automatic altitude control yet.
  ctrl_signal->thrust = my_mult( parameters_pid->altitude_p, setpoint->z_vel );

  if ( nr_motors == 4 )
  {
    control_allocation_quad_x( ctrl_signal, motor_setpoint );
  }

}

void control_allocation_quad_x( control_signal_t *ctrl_signal, int32_t motor_setpoint[] )
{
  motor_setpoint[0] = ((int32_t) (ctrl_signal->thrust - ctrl_signal->torque_roll / 4 - ctrl_signal->torque_pitch / 4 + ctrl_signal->torque_yaw / 4));
  motor_setpoint[1] = ((int32_t) (ctrl_signal->thrust + ctrl_signal->torque_roll / 4 - ctrl_signal->torque_pitch / 4 - ctrl_signal->torque_yaw / 4));
  motor_setpoint[2] = ((int32_t) (ctrl_signal->thrust + ctrl_signal->torque_roll / 4 + ctrl_signal->torque_pitch / 4 + ctrl_signal->torque_yaw / 4));
  motor_setpoint[3] = ((int32_t) (ctrl_signal->thrust - ctrl_signal->torque_roll / 4 + ctrl_signal->torque_pitch / 4 - ctrl_signal->torque_yaw / 4));

  int i;
  for ( i = 0; i < 4; i++ )
  {
    if ( motor_setpoint[i] < 0 )
    {
      motor_setpoint[i] = 0;
    }
  }
}

void limit_value( int32_t *val, int32_t limit )
{
  if ( *val > limit )
  {
    *val = limit;
  }
  else if ( *val < -limit )
  {
    *val = -limit;
  }
}
