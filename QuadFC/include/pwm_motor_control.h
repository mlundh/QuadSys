/*
 * pwm_motor_control.h
 *
 *  Created on: Mar 23, 2014
 *      Author: martin
 */

#ifndef PWM_MOTOR_CONTROL_H_
#define PWM_MOTOR_CONTROL_H_

#include "asf.h"

 //TODO Make parameters!
	/** Required resolution = 2000 usable steps. */

    /** PWM frequency in Hz */
    #define PWM_FREQUENCY  (400)
    /** PWM period value */
    #define PERIOD_VALUE   (7100)
    /** Initial duty cycle value */
    #define INIT_DUTY_VALUE  (3100)
    /** Initial dead time value */
    #define INIT_DEAD_TIME   (0)
    /** Maximum synchronous update period */
    #define MAX_SYNC_UPDATE_PERIOD  (0)
	/** Minimum input over INIT_DUTY_VALUE for the propellers to start rotating */
	#define MIN_START_DUTY (200)
	/**  Max allowed setponit */
	#define MAX_SETPOINT (5680)
/* Contains identification information required for accessing a pwm channel. */
typedef struct quad_pwm_parameters {
	uint32_t pwm_gipo;					/*< The pin of the channel */
	uint32_t pwm_flags;					/*< The flags used to initialize the pin */
	uint32_t pwm_channel;				/*< The channel of the pwm. */
} quad_pwm_parameters_t;



#endif /* PWM_MOTOR_CONTROL_H_ */
