/*
 * motor_control.h
 *
 *  Created on: 26 mar 2014
 *      Author: martin.lundh
 */

#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

 uint8_t pwm_update_setpoint(int32_t setpoint[], uint32_t nr_motors);
 uint8_t pwm_init_motor_control(uint32_t nr_motors);
 void pwm_enable();
 void pwm_dissable();

 uint8_t mk_esc_init_motor_control(uint32_t nr_motors);
 uint8_t mk_esc_update_setpoint(int32_t setpoint[], uint32_t nr_motors);
 void mk_esc_enable();
 void mk_esc_dissable();


#endif /* MOTOR_CONTROL_H_ */
