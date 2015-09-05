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
#include "pid.h"
#include "FreeRTOS.h"
#include "my_math.h"

struct pidInternal
{
  int32_t lastMeasurement;    //!<Last measurment is saved, shifted by SHIFT_EXP.
  int32_t Iterm;              //!<Iterm is saved, shifted by SHIFT_EXP.
  Pid_mode_t mode;            //!<Mode of the pid, on or off.
};
/**
 * Internal function used to limit the Iterm and output
 * of the PID controller according to the configured
 * min and max limits.
 * @param pidConfig     PID controller object.
 */
void Pid_LimitOutputAndIterm(pidConfig_t *pidConfig, int32_t *output, int32_t shiftedOutput);

/**
 * Initialization function used to initialize the PID controller
 * so that no sudden increase in control action occurs when the
 * controller is first initialized.
 * @param pidConfig     PID controller object.
 * @param output        Current output from alternative controller methods.
 * @param Measurement   Measurements from sensors.
 */
void Pid_Initialize(pidConfig_t *pidConfig, int32_t output, int32_t Measurement);


pidConfig_t* Pid_Create(int32_t kp, int32_t ki, int32_t kd, int32_t OutMin, int32_t OutMax)
{
  pidConfig_t * ptr = pvPortMalloc(sizeof(pidConfig_t));
  ptr->kp = kp;
  ptr->ki = ki;
  ptr->kd = kd;
  ptr->OutMin = OutMin;
  ptr->OutMax = OutMax;
  ptr->shift_factor = 6;

  ptr->internal = pvPortMalloc(sizeof(pidInternal_t));
  ptr->internal->lastMeasurement = 0;
  ptr->internal->Iterm = 0;
  ptr->internal->mode = pid_off;

  return ptr;
}

/*Possibly add a feed-forward term.*/
//TODO check fixed point handling.

void Pid_Compute(pidConfig_t *pidConfig, int32_t Measurement, int32_t Setpoint, int32_t *output)
{
  if(pid_off == pidConfig->internal->mode)
  {
    return;
  }
  // Preparation for fixed point calculation.
  Measurement = Measurement << pidConfig->shift_factor;
  Setpoint = Setpoint << pidConfig->shift_factor;

  /*Compute all the working error variables*/
   int32_t error = Setpoint - Measurement;

   /*Save (ki * error) in Iterm to ensure that a change
    * in ki does not create a bump in the control action.
    */
   pidConfig->internal->Iterm += my_mult2(pidConfig->ki, error, pidConfig->shift_factor);

   /*Use derivative of measurement to avoid derivative kick.*/
   int32_t dInput = (Measurement - pidConfig->internal->lastMeasurement);

   /*Remember some variables for next time*/
   pidConfig->internal->lastMeasurement = Measurement;

   /*Compute PID Output*/
   int32_t shiftedOutput = my_mult2(pidConfig->kp, error, pidConfig->shift_factor)
                           + (pidConfig->internal->Iterm)
                           - my_mult2(pidConfig->kd, dInput, pidConfig->shift_factor);

   /*Shift output back to its correct range.*/
   *output = shiftedOutput >> pidConfig->shift_factor;
   /*Make sure the output stays within the given range.*/
   Pid_LimitOutputAndIterm(pidConfig, output, shiftedOutput);
}

void Pid_LimitOutputAndIterm(pidConfig_t *pidConfig, int32_t *output, int32_t shiftedOutput)
{
   if(*output > pidConfig->OutMax)
   {
     *output = pidConfig->OutMax;
     // internal->Iterm is shifted thus outMax has to be shifted to.
     pidConfig->internal->Iterm -= (shiftedOutput  - (pidConfig->OutMax << pidConfig->shift_factor));
   }
   else if(*output < pidConfig->OutMin)
   {
     *output = pidConfig->OutMin;
     // internal->Iterm is shifted thus outMin has to be shifted to.
     pidConfig->internal->Iterm += ((pidConfig->OutMin << pidConfig->shift_factor) - shiftedOutput);
   }
}


void Pid_Initialize(pidConfig_t *pidConfig, int32_t output, int32_t Measurement)
{
  pidConfig->internal->lastMeasurement = Measurement;
  pidConfig->internal->Iterm = output;
  Pid_LimitOutputAndIterm(pidConfig, &output, output << pidConfig->shift_factor);
}

void Pid_SetMode(pidConfig_t *pidConfig, Pid_mode_t Mode, int32_t output, int32_t Measurement)
{
    if((Mode == pid_on) && (pidConfig->internal->mode != pid_on))
    {  /*we just went from off to on*/
      Pid_Initialize(pidConfig, output, Measurement);
    }
    pidConfig->internal->mode = Mode;
}

