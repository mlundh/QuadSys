/*
 * control_algorithm.h
 *
 * Created: 2013-06-18 16:47:21
 *  Author: Martin Lundh
 */

#ifndef CONTROL_ALGORITHM_H_
#define CONTROL_ALGORITHM_H_

#include "control_types.h"
#include "compiler.h"

void calc_control_signal_angle_pid( int32_t motor_setpoint[], uint32_t nr_motors, control_values_pid_t *parameters_pid, control_values_pid_t *ctrl_error, state_data_t *state,
    state_data_t *setpoint, control_signal_t *ctrl_signal );
void calc_control_signal_rate_pid( int32_t motor_setpoint[], uint32_t nr_motors, control_values_pid_t *parameters_pid, control_values_pid_t *ctrl_error, state_data_t *state,
    state_data_t *setpoint, control_signal_t *ctrl_signal );
void control_allocation_quad_x( control_signal_t *ctrl_signal, int32_t motor_setpoint[] );

#endif /* CONTROL_ALGORITHM_H_ */
