/*
 * PWM_motor_control
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
 
#include "pwm_motor_control.h"

static 	pwm_channel_t sync_channel = {
			/* Use PWM clock A as source clock */
			.ul_prescaler = PWM_CMR_CPRE_CLKA,
			/* Period value of output waveform */
			.ul_period = PERIOD_VALUE,
			/* Duty cycle value of output waveform */
			.ul_duty = INIT_DUTY_VALUE,
			/* Set it as a synchronous channel */
			.b_sync_ch = true,
			/* Enable dead-time generator */
			.b_deadtime_generator = false,
			/* Dead-time value for PWMH output */
			.us_deadtime_pwmh = INIT_DEAD_TIME,
			/* Dead-time value for PWML output */
			.us_deadtime_pwml = INIT_DEAD_TIME,
			/* Disable override PWMH outputs */
			.output_selection.b_override_pwmh = false,
			/* Disable override PWML outputs */
			.output_selection.b_override_pwml = false
		};

static const quad_pwm_parameters_t pwm_all_pwm_parameters[8] =
{
		{PIN_6_PWM_GPIO, PIN_6_PWM_FLAGS, PIN_6_PWM_CHANNEL},
		{PIN_7_PWM_GPIO, PIN_7_PWM_FLAGS, PIN_7_PWM_CHANNEL},
		{PIN_8_PWM_GPIO, PIN_8_PWM_FLAGS, PIN_8_PWM_CHANNEL},
		{PIN_9_PWM_GPIO, PIN_9_PWM_FLAGS, PIN_9_PWM_CHANNEL},
		{PIN_34_PWM_GPIO, PIN_34_PWM_FLAGS, PIN_34_PWM_CHANNEL},
		{PIN_36_PWM_GPIO, PIN_36_PWM_FLAGS, PIN_36_PWM_CHANNEL},
		{PIN_38_PWM_GPIO, PIN_38_PWM_FLAGS, PIN_38_PWM_CHANNEL},
		{PIN_40_PWM_GPIO, PIN_40_PWM_FLAGS, PIN_40_PWM_CHANNEL}


};

static uint32_t nr_init_motors = 0;

 uint8_t pwm_init_motor_control(uint32_t nr_motors)
 {
	 /*make sure that the correct number of motors are used.*/
	 if((nr_motors < 0) || (nr_motors > 8))
	 {
		 return 1;
	 }



	 //TODO read sync_channel parameters from flash.


	 /* Enable PWM peripheral clock */

    pmc_enable_periph_clk(ID_PWM);

	for(int i = 0; i < nr_motors; i++)
	{
		pwm_channel_disable(PWM, pwm_all_pwm_parameters[i].pwm_channel);
	}

	/*
	 * In PWM synchronization mode the channel0 is used as reference channel,
	 * so it is necessary to disable, configure and enable it.
	 */

	pwm_channel_disable(PWM, 0);


	/* Set PWM clock A as PWM_FREQUENCY*PERIOD_VALUE (clock B is not used) */
	pwm_clock_t clock_setting = {
		.ul_clka = PWM_FREQUENCY * PERIOD_VALUE,
		.ul_clkb = 0,
		.ul_mck = sysclk_get_cpu_hz()
	};
	pwm_init(PWM, &clock_setting);

	/*
	 * In PWM synchronization mode the channel0 is used as reference channel,
	 * so it is necessary to disable, configure and enable it.
	 */
	sync_channel.channel = 0;
	pwm_channel_init(PWM, &sync_channel);


	for(int i = 0; i < nr_motors; i++)
	{
		sync_channel.channel = pwm_all_pwm_parameters[i].pwm_channel;
		pwm_channel_init(PWM, &sync_channel);
	}

	/*
	 * Initialize PWM synchronous channels
	 * Synchronous Update Mode: manually update duty cycle value. The
	 * update occurs next pwm cycle (MODE 0).
	 *
	 * Synchronous Update Period = MAX_SYNC_UPDATE_PERIOD.
	 */
	pwm_sync_init(PWM, PWM_SYNC_UPDATE_MODE_0, MAX_SYNC_UPDATE_PERIOD);

	nr_init_motors = nr_motors;
	return 0;
 }

 void pwm_enable()
 {
	/* Enable all synchronous channels by enabling channel 0 */
	 pwm_channel_enable(PWM, 0);
 }

 void pwm_dissable()
 {
	 pwm_channel_disable(PWM, 0);
 }

 uint8_t pwm_update_setpoint(int32_t setpoint[], uint32_t nr_motors)
 {
	 if(nr_motors != nr_init_motors)
	 {
		 return 1;
	 }
	 for(int i = 0; i < nr_motors; i++)
	 {
		 sync_channel.channel = pwm_all_pwm_parameters[i].pwm_channel;
		 pwm_channel_update_duty(PWM, &sync_channel, (uint32_t)setpoint[i] + INIT_DUTY_VALUE);
	 }
	 pwm_sync_unlock_update(PWM);
	 return 0;
 }
