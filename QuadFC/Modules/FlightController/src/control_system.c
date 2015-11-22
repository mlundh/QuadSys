/*
 * control_system.c
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


#include "stddef.h"
#include "control_system.h"
#include "pid.h"
#include "control_mode_handler.h"
#include "parameters.h"
#include "led_control_task.h" // TODO refactor LED

struct CtrlObj
{
  pidConfig_t* RatePitch;
  pidConfig_t* RateRoll;
  pidConfig_t* RateYaw;
  SemaphoreHandle_t xMutexParam;
  CtrlModeHandler_t* ModeHandler;
};

void Ctrl_Initialize(CtrlObj_t *internals);


CtrlObj_t *Ctrl_Create()
{
  CtrlObj_t *internals = pvPortMalloc(sizeof(struct CtrlObj));
  internals->ModeHandler = Ctrl_CreateModeHandler();

  return internals;
}

uint8_t Ctrl_init(CtrlObj_t *obj)
{
  Ctrl_Initialize(obj);
  Ctrl_InitModeHandler(obj->ModeHandler);
  return 1;
}

void Ctrl_Initialize(CtrlObj_t *internals)
{

  internals->RatePitch = Pid_Create(220, 3, 0, -2000, 2000);
  internals->RateRoll = Pid_Create(220, 3, 0, -2000, 2000);
  internals->RateYaw = Pid_Create(220, 3, 0, -2000, 2000);

  internals->xMutexParam = xSemaphoreCreateMutex();

  param_obj_t * RateRoot = Param_CreateObj(10, NoType, readOnly, NULL, "PID_R", Param_GetRoot(), NULL);

  // Enable tuning of the pid parameters.
  param_obj_t * pitchObj = Param_CreateObj(10, NoType, readOnly, NULL, "Pitch", RateRoot, NULL);
  param_obj_t * rollObj = Param_CreateObj(10, NoType, readOnly, NULL, "Roll", RateRoot, NULL);
  param_obj_t * yawObj = Param_CreateObj(10, NoType, readOnly, NULL, "Yaw", RateRoot, NULL);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RatePitch->kp, "Kp", pitchObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RatePitch->ki, "Ki", pitchObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RatePitch->kd, "Kd", pitchObj, internals->xMutexParam);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateRoll->kp, "Kp", rollObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateRoll->ki, "Ki", rollObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateRoll->kd, "Kd", rollObj, internals->xMutexParam);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateYaw->kp, "Kp", yawObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateYaw->ki, "Ki", yawObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateYaw->kd, "Kd", yawObj, internals->xMutexParam);


  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RatePitch->OutMax, "OutMax", pitchObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RatePitch->OutMin, "OutMin", pitchObj, internals->xMutexParam);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateRoll->OutMax, "OutMax", rollObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateRoll->OutMin, "OutMin", rollObj, internals->xMutexParam);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateYaw->OutMax, "OutMax", yawObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateYaw->OutMin, "OutMin", yawObj, internals->xMutexParam);


}


void Ctrl_Execute(CtrlObj_t *internals, state_data_t *state, state_data_t *setpoint, control_signal_t *u_signal)
{
  // Make sure that we are not trying to update the pid parameters while using them.
  if( xSemaphoreTake( internals->xMutexParam, ( TickType_t )(1UL / portTICK_PERIOD_MS) ) == pdTRUE )
  {
    switch (Ctrl_GetCurrentMode(internals->ModeHandler))
    {
    case Control_mode_rate:
      /*-----------------------------Rate mode-----------------------
       *
       */
      Pid_Compute(internals->RatePitch, state->state_vector[pitch_rate],
          setpoint->state_vector[pitch_rate],
          &(u_signal->control_signal[u_pitch]));

      Pid_Compute(internals->RateRoll, state->state_vector[roll_rate],
          setpoint->state_vector[roll_rate],
          &(u_signal->control_signal[u_roll]));

      Pid_Compute(internals->RateYaw, state->state_vector[yaw_rate],
          setpoint->state_vector[yaw_rate],
          &(u_signal->control_signal[u_yaw]));
      u_signal->control_signal[u_thrust] = setpoint->state_vector[z_vel];
      break;
    case Control_mode_attitude:
      /*-----------------------------Attitude mode-------------------
       *
       */

      break;
    case Control_mode_not_available:
      /*----------------------------Mode not available------------
       * Something is very wrong, we have an unknown state.
       */
      break;
    default:
      break;
    }
    xSemaphoreGive(internals->xMutexParam);
  }
  else
  {
    Led_Set(led_main_blocked);

  }
}

void Ctrl_On(CtrlObj_t * param)
{
  Pid_SetMode(param->RatePitch, pid_on, 0, 0);
  Pid_SetMode(param->RateRoll, pid_on, 0, 0);
  Pid_SetMode(param->RateYaw, pid_on, 0, 0);
}

void Ctrl_Off(CtrlObj_t * param)
{
  Pid_SetMode(param->RatePitch, pid_off, 0, 0);
  Pid_SetMode(param->RateRoll, pid_off, 0, 0);
  Pid_SetMode(param->RateYaw, pid_off, 0, 0);
}

void Ctrl_Allocate( control_signal_t *ctrl_signal, int32_t motor_setpoint[] )
{
  motor_setpoint[0] = ((int32_t) ( ctrl_signal->control_signal[u_thrust] -  ctrl_signal->control_signal[u_roll] / 4 - ctrl_signal->control_signal[u_pitch] / 4 + ctrl_signal->control_signal[u_yaw] / 4));
  motor_setpoint[1] = ((int32_t) ( ctrl_signal->control_signal[u_thrust] +  ctrl_signal->control_signal[u_roll] / 4 - ctrl_signal->control_signal[u_pitch] / 4 - ctrl_signal->control_signal[u_yaw] / 4));
  motor_setpoint[2] = ((int32_t) ( ctrl_signal->control_signal[u_thrust] +  ctrl_signal->control_signal[u_roll] / 4 + ctrl_signal->control_signal[u_pitch] / 4 + ctrl_signal->control_signal[u_yaw] / 4));
  motor_setpoint[3] = ((int32_t) ( ctrl_signal->control_signal[u_thrust] -  ctrl_signal->control_signal[u_roll] / 4 + ctrl_signal->control_signal[u_pitch] / 4 - ctrl_signal->control_signal[u_yaw] / 4));

  int i;
  for ( i = 0; i < 4; i++ )
  {
    if ( motor_setpoint[i] < 0 )
    {
      motor_setpoint[i] = 0;
    }
  }
}

