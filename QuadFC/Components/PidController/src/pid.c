/*
 * pid.c
 *
 * Copyright (C) 2015 Martin Lundh
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
#include "../inc/pid.h"
#include "FreeRTOS.h"
#include "Utilities/inc/my_math.h"

struct pidInternal
{
  int32_t lastMeasurement;    //!<Last measurment is saved.
  int32_t IError;             //!<Iterm is saved.
  Pid_mode_t mode;            //!<Mode of the pid, on or off.
};

/**
 * Internal function for limiting the I term summation. It will
 * Clamp the I term to the max value.
 * @param pidConfig
 */
void Pid_LimitIterm(pidConfig_t *pidConfig);

/**
 * Internal function used to limit the output
 * of the PID controller according to the configured
 * min and max limits.
 * @param pidConfig     PID controller object.
 */
void Pid_LimitOutput(pidConfig_t *pidConfig, int32_t *output);

/**
 * Initialization function used to initialize the PID controller
 * so that no sudden increase in control action occurs when the
 * controller is first initialized.
 * @param pidConfig     PID controller object.
 * @param output        Current output from alternative controller methods.
 * @param Measurement   Measurements from sensors.
 */
void Pid_Initialize(pidConfig_t *pidConfig, int32_t output, int32_t Measurement);


pidConfig_t* Pid_Create(int32_t kp, int32_t ki, int32_t kd, int32_t OutMin, int32_t OutMax, uint32_t shiftFactor, uint32_t timeInterval)
{
  pidConfig_t * ptr = pvPortMalloc(sizeof(pidConfig_t));
  if(!ptr)
  {
    return NULL;
  }
  ptr->kp = kp;
  ptr->ki = ki;
  ptr->kd = kd;
  ptr->OutMin = OutMin;
  ptr->OutMax = OutMax;
  ptr->shift_factor = shiftFactor;
  ptr->timeInterval = timeInterval;

  ptr->internal = pvPortMalloc(sizeof(pidInternal_t));
  if(!ptr->internal)
  {
    return NULL;
  }
  ptr->internal->lastMeasurement = 0;
  ptr->internal->IError = 0;
  ptr->internal->mode = pid_off;

  return ptr;
}

/*Possibly add a feed-forward term.*/

void Pid_Compute(pidConfig_t *pidConfig, int32_t Measurement, int32_t Setpoint, int32_t *output)
{
  if(pid_off == pidConfig->internal->mode)
  {
    return;
  }

  /*Compute all the working error variables*/
   int32_t error = Setpoint - Measurement;

   /*Save (ki * error * timeInterval) in Iterm to ensure that a change
    * in ki does not create a bump in the control action.
    */
   pidConfig->internal->IError += my_mult(pidConfig->ki, my_mult(error, pidConfig->timeInterval, pidConfig->shift_factor), pidConfig->shift_factor);
   // Limit I term.
   Pid_LimitIterm(pidConfig);


   /*Use derivative of measurement to avoid derivative kick. dError = (error - last_error) / timeInterval*/
   int32_t dError = my_div((Measurement - pidConfig->internal->lastMeasurement), pidConfig->timeInterval, pidConfig->shift_factor);

   /*Remember some variables for next time*/
   pidConfig->internal->lastMeasurement = Measurement;

   /*Compute PID Output*/
   *output = my_mult(pidConfig->kp, error, pidConfig->shift_factor)
                           + (pidConfig->internal->IError)
                           - my_mult(pidConfig->kd, dError, pidConfig->shift_factor);

   /*Make sure the output stays within the given range.*/
   Pid_LimitOutput(pidConfig, output);
}

void Pid_LimitIterm(pidConfig_t *pidConfig)
{
  if(pidConfig->internal->IError > pidConfig->OutMax)
  {
    pidConfig->internal->IError = pidConfig->OutMax;
  }
  else if(pidConfig->internal->IError < pidConfig->OutMin)
  {
    pidConfig->internal->IError = pidConfig->OutMin;
  }
}

void Pid_LimitOutput(pidConfig_t *pidConfig, int32_t *output)
{
   if(*output > pidConfig->OutMax)
   {
     *output = pidConfig->OutMax;
   }
   else if(*output < pidConfig->OutMin)
   {
     *output = pidConfig->OutMin;
   }
}


void Pid_Initialize(pidConfig_t *pidConfig, int32_t output, int32_t Measurement)
{
  pidConfig->internal->lastMeasurement = Measurement;
  if(pidConfig->ki == 0)
  {
    pidConfig->internal->IError = 0;
  }
  else
  {
    pidConfig->internal->IError = output;
  }
  Pid_LimitOutput(pidConfig, &output);
}

void Pid_SetMode(pidConfig_t *pidConfig, Pid_mode_t Mode, int32_t output, int32_t Measurement)
{
    if((Mode == pid_on) && (pidConfig->internal->mode != pid_on))
    {  /*we just went from off to on*/
      Pid_Initialize(pidConfig, output, Measurement);
    }
    pidConfig->internal->mode = Mode;
}

void Pid_UpdateLimits(pidConfig_t *pidConfig, int32_t min, int32_t max)
{
  pidConfig->OutMax = max;
  pidConfig->OutMin = min;
}