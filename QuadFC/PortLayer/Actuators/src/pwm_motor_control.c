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

#include "QuadFC/QuadFC_MotorControl.h"
#include <pwm.h>
#include <pio.h>
#include <board.h>
#include <sysclk.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>
#include "Parameters/inc/parameters.h"
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

/**
 *  Contains identification information
 *  required for accessing a pwm channel.
 *
 */
typedef struct quad_pwm_parameters
{
  uint32_t pwm_gipo; /*< The pin of the channel */
  uint32_t pwm_flags; /*< The flags used to initialize the pin */
  uint32_t pwm_channel; /*< The channel of the pwm. */
} quad_pwm_parameters_t;

typedef struct pwm_internals
{
  quad_pwm_parameters_t *pwm_parameters;
  pwm_channel_t pwm_channels;
  uint32_t arming_duty_value;
  uint32_t armed_duty_value;
  SemaphoreHandle_t xMutex;
} pwm_internals_t;

/**
 * Create a pwm motor control object.
 * @return
 */
pwm_internals_t *pwm_create();

/**
 * Convenience function to get a pointer to the internals struct.
 * @param obj   Current motor control object.
 * @return      Pointer to internals struct.
 */
pwm_internals_t *pwm_getInternals(MotorControlObj *obj);

/**
 * Convenience function to take the mutex assiciated with the current object.
 * @param obj   Current internals object.
 * @return      0 if failed to aquire mutex(do not use resource) 1 otherwise.
 */
uint8_t pwm_takeMutex(pwm_internals_t *obj);

/**
 * Convenience function to release the mutex assiciated with the current object.
 * @param obj   Current internals object.
 * @return      0 if failed, 1 otherwise.
 */
uint8_t pwm_giveMutex(pwm_internals_t *obj);


#define MAX_MOTORS (8)

pwm_internals_t *pwm_create()
{
  pwm_internals_t *internals = pvPortMalloc(sizeof(pwm_internals_t));
  if(!internals)
  {
    return 0;
  }

  internals->arming_duty_value = INIT_DUTY_VALUE;
  internals->armed_duty_value = MIN_START_DUTY + INIT_DUTY_VALUE;

  /* Use PWM clock A as source clock */
  internals->pwm_channels.ul_prescaler = PWM_CMR_CPRE_CLKA;
  /* Period value of output waveform */
  internals->pwm_channels.ul_period = PERIOD_VALUE;
  /* Duty cycle value of output waveform */
  internals->pwm_channels.ul_duty = INIT_DUTY_VALUE;
  /* Set it as a synchronous channel */
  internals->pwm_channels.b_sync_ch = true;
  /* Enable dead-time generator */
  internals->pwm_channels.b_deadtime_generator = false;
  /* Dead-time value for PWMH output */
  internals->pwm_channels.us_deadtime_pwmh = INIT_DEAD_TIME;
  /* Dead-time value for PWML output */
  internals->pwm_channels.us_deadtime_pwml = INIT_DEAD_TIME;
  /* Disable override PWMH outputs */
  internals->pwm_channels.output_selection.b_override_pwmh = false;
  /* Disable override PWML outputs */
  internals->pwm_channels.output_selection.b_override_pwml = false;


  // Initialize and assign the pwm pins. This is done in a temporary struct.
  quad_pwm_parameters_t pwm_all_pwm_parameters[] =
      {
          { PIN_34_PWM_GPIO, PIN_34_PWM_FLAGS, PIN_34_PWM_CHANNEL },
          { PIN_36_PWM_GPIO, PIN_36_PWM_FLAGS, PIN_36_PWM_CHANNEL },
          { PIN_38_PWM_GPIO, PIN_38_PWM_FLAGS, PIN_38_PWM_CHANNEL },
          { PIN_40_PWM_GPIO, PIN_40_PWM_FLAGS, PIN_40_PWM_CHANNEL },
          { PIN_6_PWM_GPIO, PIN_6_PWM_FLAGS, PIN_6_PWM_CHANNEL },
          { PIN_7_PWM_GPIO, PIN_7_PWM_FLAGS, PIN_7_PWM_CHANNEL },
          { PIN_8_PWM_GPIO, PIN_8_PWM_FLAGS, PIN_8_PWM_CHANNEL },
          { PIN_9_PWM_GPIO, PIN_9_PWM_FLAGS, PIN_9_PWM_CHANNEL }
      };
  internals->pwm_parameters = pvPortMalloc(sizeof(pwm_all_pwm_parameters));
  if(!internals->pwm_parameters)
  {
    return NULL;
  }
  //Copy the temporary array of structs.
  memcpy(internals->pwm_parameters, &pwm_all_pwm_parameters, sizeof(pwm_all_pwm_parameters));

  return internals;
}





MotorControlObj * MotorCtrl_CreateAndInit(uint32_t nr_motors )
{
  /*make sure that the correct number of motors are used.*/
  if ( (nr_motors < 0) || (nr_motors > MAX_MOTORS) )
  {
    return NULL;
  }


  //Create
  MotorControlObj *obj = pvPortMalloc(sizeof(MotorControlObj));
  if(!obj)
  {
    return NULL;
  }
  obj->armed = 0;
  obj->controlInternals = (void *)pwm_create();
  obj->nr_init_motors = 0;
  obj->motorSetpoint = pvPortMalloc(sizeof(int32_t) * nr_motors);

  if(!obj->controlInternals || !obj->motorSetpoint)
  {
    return NULL;
  }

  memset(obj->motorSetpoint, 0, sizeof(int32_t) * nr_motors);

  // Enable tuning of parameters.

  pwm_internals_t *internals = pwm_getInternals(obj);
  internals->xMutex = xSemaphoreCreateMutex();
  param_obj_t * PwmRoot = Param_CreateObj(10, NoType, readOnly, NULL, "PWM", Param_GetRoot(), NULL);

  Param_CreateObj(0, uint32_variable_type, readWrite,
      &internals->armed_duty_value, "dtyOn", PwmRoot, internals->xMutex);
  Param_CreateObj(0, uint32_variable_type, readWrite,
      &internals->arming_duty_value, "dtyArm", PwmRoot, internals->xMutex);


  //Init

  //TODO read sync_channel parameters from flash.

  /* Enable PWM peripheral clock */

  pmc_enable_periph_clk( ID_PWM );

  for ( int i = 0; i < nr_motors; i++ )
  {
    if ( internals->pwm_parameters[i].pwm_channel != 0 )
    {
      pwm_channel_disable( PWM, internals->pwm_parameters[i].pwm_channel );
    }
  }

  /*
   * In PWM synchronization mode the channel0 is used as reference channel,
   * so it is necessary to disable, configure and enable it.
   */

  pwm_channel_disable( PWM, 0 );

  /* Set PWM clock A as PWM_FREQUENCY*PERIOD_VALUE (clock B is not used) */
  pwm_clock_t clock_setting = {
      .ul_clka = PWM_FREQUENCY * PERIOD_VALUE,
      .ul_clkb = 0,
      .ul_mck = sysclk_get_cpu_hz()
  };
  pwm_init( PWM, &clock_setting );

  /*
   * In PWM synchronization mode the channel0 is used as reference channel,
   * so it is necessary to disable, configure and enable it.
   */
  internals->pwm_channels.channel = 0;
  pwm_channel_init( PWM, &internals->pwm_channels );

  for ( int i = 0; i < nr_motors; i++ )
  {
    if ( internals->pwm_parameters[i].pwm_channel != 0 )
    {
      internals->pwm_channels.channel = internals->pwm_parameters[i].pwm_channel;
      pwm_channel_init( PWM, &internals->pwm_channels );
    }
  }

  /*
   * Initialize PWM synchronous channels
   * Synchronous Update Mode: manually update duty cycle value. The
   * update occurs next pwm cycle (MODE 0).
   *
   * Synchronous Update Period = MAX_SYNC_UPDATE_PERIOD.
   */
  pwm_sync_init( PWM, PWM_SYNC_UPDATE_MODE_0, MAX_SYNC_UPDATE_PERIOD );

  obj->nr_init_motors = nr_motors;


  return obj;
}

uint8_t MotorCtrl_Enable(MotorControlObj *obj)
{
  if(!obj)
  {
    return 0;
  }
  pwm_internals_t *internals = pwm_getInternals(obj);
  /* Enable all synchronous channels by enabling channel 0 */

  // Make sure we are thread safe.
  if(!pwm_takeMutex(internals))
  {
    return 0;
  }

  pwm_channel_enable( PWM, 0 );

  for ( int i = 0; i < obj->nr_init_motors; i++ )
  {
    internals->pwm_channels.channel = internals->pwm_parameters[i].pwm_channel;
    pwm_channel_update_duty( PWM, &internals->pwm_channels, internals->arming_duty_value );
  }
  pwm_sync_unlock_update( PWM );
  obj->armed = 1;

  // Make sure we are thread safe.
  pwm_giveMutex(internals);
  return 1;
}

uint8_t MotorCtrl_Disable(MotorControlObj *obj)
{
  pwm_channel_disable( PWM, 0 );
  obj->armed = 0;
  return 1;
}

uint8_t MotorCtrl_UpdateSetpoint(MotorControlObj *obj)
{

  pwm_internals_t *internals = pwm_getInternals(obj);

  // Make sure we are thread safe.
  if(!pwm_takeMutex(internals))
  {
    return 0;
  }


  for (int i = 0; i < obj->nr_init_motors; i++ )
  {
    /*Add armed duty value to make to propellers spin.*/
    obj->motorSetpoint[i] += internals->armed_duty_value;
    
    if ( obj->motorSetpoint[i] < internals->armed_duty_value )
    {
      obj->motorSetpoint[i] = internals->armed_duty_value;
    }
    else if ( obj->motorSetpoint[i] > MAX_SETPOINT )
		{
      obj->motorSetpoint[i] = MAX_SETPOINT;
		}
	}
	 for(int i = 0; i < obj->nr_init_motors; i++)
	 {
	   internals->pwm_channels.channel = internals->pwm_parameters[i].pwm_channel;
		 pwm_channel_update_duty(PWM, &internals->pwm_channels, (obj->motorSetpoint[i]));
	 }
	 pwm_sync_unlock_update(PWM);

	 // Make sure we are thread safe.
	 pwm_giveMutex(internals);
	 return 1;
}

pwm_internals_t *pwm_getInternals(MotorControlObj *obj)
{
  return (pwm_internals_t *)obj->controlInternals;
}

uint8_t pwm_takeMutex(pwm_internals_t *obj)
{
  if(obj->xMutex)
  {
    if( !xSemaphoreTake( obj->xMutex, ( TickType_t )(2UL / portTICK_PERIOD_MS) ) == pdTRUE )
    {
      return 0; // Was not able to aquire mutex, return with error.
    }
  }
  return 1;
}
uint8_t pwm_giveMutex(pwm_internals_t *obj)
{
  if(obj->xMutex)
  {
    xSemaphoreGive(obj->xMutex);
  }
  return 1;
}
