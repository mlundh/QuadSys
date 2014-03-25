/*
 * pwm_motor_control.h
 *
 *  Created on: Mar 23, 2014
 *      Author: martin
 */

#ifndef PWM_MOTOR_CONTROL_H_
#define PWM_MOTOR_CONTROL_H_


 //TODO Wrong place!  Make parameters!
    /** PWM frequency in Hz */
    #define PWM_FREQUENCY  400
    /** PWM period value */
    #define PERIOD_VALUE   3100
    /** Initial duty cycle value */
    #define INIT_DUTY_VALUE  1100
    /** Initial dead time value */
    #define INIT_DEAD_TIME   0
    /** Maximum synchronous update period */
    #define MAX_SYNC_UPDATE_PERIOD  0

/* Contains identification information required for accessing a pwm channel. */
typedef struct quad_pwm_parameters {
	uint32_t pwm_gipo;					/*< The pin of the channel */
	uint32_t pwm_flags;					/*< The flags used to initialize the pin */
	uint32_t pwm_channel;				/*< The channel of the pwm. */
} quad_pwm_parameters_t;



 uint8_t pwm_update_setpoint(int32_t setpoint[], uint32_t nr_motors);
 void pwm_enable();
 void pwm_dissable();
 uint8_t init_pwm_motor_control(uint32_t nr_motors);


#endif /* PWM_MOTOR_CONTROL_H_ */
