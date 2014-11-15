/*
 * main_control_task.h
 *
 * Created: 2013-03-29 19:07:05
 *  Author: Martin Lundh
 */

#ifndef MAIN_CONTROL_TASK_H_
#define MAIN_CONTROL_TASK_H_
#include "control_types.h"


extern control_values_pid_t *parameters_rate;
extern state_data_t *state;
extern state_data_t *setpoint;
extern control_signal_t *ctrl_signal;


// Global log parameters
extern int32_t **log_parameters;
extern uint8_t nr_log_parameters;
extern uint32_t log_freq;
extern int32_t time_main;

void create_main_control_task( void );


#endif /* MAIN_CONTROL_TASK_H_ */
