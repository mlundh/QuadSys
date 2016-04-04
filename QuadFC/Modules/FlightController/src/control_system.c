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


#include <stddef.h>
#include "../inc/control_system.h"
#include "../inc/pid.h"
#include "../inc/control_mode_handler.h"
#include "Parameters/inc/parameters.h"
#include "HMI/inc/led_control_task.h" // TODO refactor LED

struct CtrlObj
{
  pidConfig_t* RatePitch;
  pidConfig_t* RateRoll;
  pidConfig_t* RateYaw;
  pidConfig_t* AttitudePitch;
  pidConfig_t* AttitudeRoll;
  pidConfig_t* AttitudeYaw;
  SemaphoreHandle_t xMutexParam;
  CtrlModeHandler_t* ModeHandler;
  CtrlMode_t current_mode;
};

uint8_t Ctrl_InitializeRate(CtrlObj_t *internals);
uint8_t Ctrl_InitializeAttitude(CtrlObj_t *internals);


CtrlObj_t *Ctrl_Create(CtrlModeHandler_t* CtrlModeHandler)
{
  CtrlObj_t *internals = pvPortMalloc(sizeof(struct CtrlObj));
  internals->ModeHandler = CtrlModeHandler;

  return internals;
}

uint8_t Ctrl_init(CtrlObj_t *obj)
{
  obj->xMutexParam = xSemaphoreCreateMutex();
  obj->current_mode = Control_mode_not_available;
  Ctrl_InitializeRate(obj);
  Ctrl_InitializeAttitude(obj);
  Ctrl_InitModeHandler(obj->ModeHandler);
  return 1;
}

uint8_t Ctrl_InitializeRate(CtrlObj_t *internals)
{
  /**
   * The constants (Kp, Ki and Kd) does not have a unit, but are expressed as 16.16 fixed point.
   * Output of the controller should lie in [0, 1<<16] where 1<<16 represent 100% control signal.
   */
  uint32_t KpInitial  = 8 << 12;
  uint32_t KiInitial  = 0 << (ANGLE_SHIFT_FACTOR-2);
  uint32_t KDInitial  = 0 << (ANGLE_SHIFT_FACTOR-4);
  uint32_t maxOutputInit = MAX_U_SIGNAL;
  uint32_t minOutputInit = -MAX_U_SIGNAL;

  internals->RatePitch = Pid_Create(KpInitial, KiInitial, KDInitial, minOutputInit, maxOutputInit, ANGLE_SHIFT_FACTOR, CTRL_TIME_FP);
  internals->RateRoll  = Pid_Create(KpInitial, KiInitial, KDInitial, minOutputInit, maxOutputInit, ANGLE_SHIFT_FACTOR, CTRL_TIME_FP);
  internals->RateYaw   = Pid_Create(KpInitial, KiInitial, KDInitial, minOutputInit, maxOutputInit, ANGLE_SHIFT_FACTOR, CTRL_TIME_FP);

  if(!internals->RatePitch || !internals->RateRoll || !internals->RateYaw)
  {
    return 0;
  }
  param_obj_t * RateRoot = Param_CreateObj(3, NoType, readOnly, NULL, "PID_R", Param_GetRoot(), NULL);

  // Enable tuning of the pid parameters.
  param_obj_t * pitchRateObj = Param_CreateObj(5, NoType, readOnly, NULL, "Pitch", RateRoot, NULL);
  param_obj_t * rollRateObj  = Param_CreateObj(5, NoType, readOnly, NULL, "Roll", RateRoot, NULL);
  param_obj_t * yawRateObj   = Param_CreateObj(5, NoType, readOnly, NULL, "Yaw", RateRoot, NULL);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RatePitch->kp, "Kp", pitchRateObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RatePitch->ki, "Ki", pitchRateObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RatePitch->kd, "Kd", pitchRateObj, internals->xMutexParam);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateRoll->kp, "Kp", rollRateObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateRoll->ki, "Ki", rollRateObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateRoll->kd, "Kd", rollRateObj, internals->xMutexParam);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateYaw->kp, "Kp", yawRateObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateYaw->ki, "Ki", yawRateObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->RateYaw->kd, "Kd", yawRateObj, internals->xMutexParam);

  return 1;
}

uint8_t Ctrl_InitializeAttitude(CtrlObj_t *internals)
{
  /**
   * The constants (Kp, Ki and Kd) does not have a unit, but are expressed as 16.16 fixed point.
   * Output of the controller should lie in [0, 1<<16] where 1<<16 represent 100% control signal.
   */
  uint32_t KpInitial  = 8 << 12;
  uint32_t KiInitial  = 0 << (ANGLE_SHIFT_FACTOR-2);
  uint32_t KDInitial  = 0 << (ANGLE_SHIFT_FACTOR-4);
  uint32_t maxOutputInit = MAX_U_SIGNAL;
  uint32_t minOutputInit = -MAX_U_SIGNAL;

  internals->AttitudePitch = Pid_Create(KpInitial, KiInitial, KDInitial, minOutputInit, maxOutputInit, ANGLE_SHIFT_FACTOR, CTRL_TIME_FP);
  internals->AttitudeRoll  = Pid_Create(KpInitial, KiInitial, KDInitial, minOutputInit, maxOutputInit, ANGLE_SHIFT_FACTOR, CTRL_TIME_FP);
  internals->AttitudeYaw   = Pid_Create(KpInitial, KiInitial, KDInitial, minOutputInit, maxOutputInit, ANGLE_SHIFT_FACTOR, CTRL_TIME_FP);

  if(!internals->AttitudePitch || !internals->AttitudeRoll || !internals->AttitudeYaw)
  {
    return 0;
  }

  param_obj_t * AttitudeRoot = Param_CreateObj(3, NoType, readOnly, NULL, "PID_A", Param_GetRoot(), NULL);

  // Enable tuning of the pid parameters.
  param_obj_t * pitchObj = Param_CreateObj(5, NoType, readOnly, NULL, "Pitch", AttitudeRoot, NULL);
  param_obj_t * rollObj  = Param_CreateObj(5, NoType, readOnly, NULL, "Roll", AttitudeRoot, NULL);
  param_obj_t * yawObj   = Param_CreateObj(5, NoType, readOnly, NULL, "Yaw", AttitudeRoot, NULL);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->AttitudePitch->kp, "Kp", pitchObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->AttitudePitch->ki, "Ki", pitchObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->AttitudePitch->kd, "Kd", pitchObj, internals->xMutexParam);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->AttitudeRoll->kp, "Kp", rollObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->AttitudeRoll->ki, "Ki", rollObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->AttitudeRoll->kd, "Kd", rollObj, internals->xMutexParam);

  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->AttitudeYaw->kp, "Kp", yawObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->AttitudeYaw->ki, "Ki", yawObj, internals->xMutexParam);
  Param_CreateObj(0, int32_variable_type, readWrite,
      &internals->AttitudeYaw->kd, "Kd", yawObj, internals->xMutexParam);

  return 1;
}

void Ctrl_Execute(CtrlObj_t *internals, state_data_t *state, state_data_t *setpoint, control_signal_t *u_signal)
{
  // Make sure that we are not trying to update the pid parameters while using them.
  if( xSemaphoreTake( internals->xMutexParam, ( TickType_t )(1UL / portTICK_PERIOD_MS) ) == pdTRUE )
  {
    CtrlMode_t newMode = Ctrl_GetCurrentMode(internals->ModeHandler);
    if(newMode != internals->current_mode)
    {
      //Change of mode required.
      Ctrl_Switch(internals, newMode, state, u_signal);
      internals->current_mode = newMode;
    }
    switch (newMode)
    {
    case Control_mode_rate:
      /*-----------------------------Rate mode-----------------------
       *
       */
      Pid_Compute(internals->RatePitch, state->state_bf[pitch_rate_bf],
          setpoint->state_bf[pitch_rate_bf],
          &(u_signal->control_signal[u_pitch]));

      Pid_Compute(internals->RateRoll, state->state_bf[roll_rate_bf],
          setpoint->state_bf[roll_rate_bf],
          &(u_signal->control_signal[u_roll]));

      Pid_Compute(internals->RateYaw, state->state_bf[yaw_rate_bf],
          setpoint->state_bf[yaw_rate_bf],
          &(u_signal->control_signal[u_yaw]));

      u_signal->control_signal[u_thrust] = setpoint->state_bf[thrust_sp];
      break;
    case Control_mode_attitude:
      /*-----------------------------Attitude mode-------------------
       *
       */
      Pid_Compute(internals->AttitudePitch, state->state_bf[pitch_bf],
          setpoint->state_bf[pitch_bf],
          &(u_signal->control_signal[u_pitch]));

      Pid_Compute(internals->AttitudeRoll, state->state_bf[roll_bf],
          setpoint->state_bf[roll_bf],
          &(u_signal->control_signal[u_roll]));
      // We do not yet have an angle estimate for the yaw angle, so we
      // use rate of yaw.
      Pid_Compute(internals->AttitudeYaw, state->state_bf[yaw_rate_bf],
          setpoint->state_bf[yaw_rate_bf],
          &(u_signal->control_signal[u_yaw]));

      u_signal->control_signal[u_thrust] = setpoint->state_bf[thrust_sp];
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
  switch (Ctrl_GetCurrentMode(param->ModeHandler))
      {
      case Control_mode_rate:
        /*-----------------------------Rate mode-----------------------
         *
         */
        Pid_SetMode(param->RatePitch, pid_on, 0, 0);
        Pid_SetMode(param->RateRoll, pid_on, 0, 0);
        Pid_SetMode(param->RateYaw, pid_on, 0, 0);
        break;
      case Control_mode_attitude:
        /*-----------------------------Attitude mode-------------------
         *
         */
        Pid_SetMode(param->AttitudePitch, pid_on, 0, 0);
        Pid_SetMode(param->AttitudeRoll, pid_on, 0, 0);
        Pid_SetMode(param->AttitudeYaw, pid_on, 0, 0);
        break;
      case Control_mode_not_available:
        /*----------------------------Mode not available------------
         * Something is very wrong, we have an unknown state.
         */
        break;
      default:
        break;
      }

}

void Ctrl_Switch(CtrlObj_t * param, CtrlMode_t newMode, state_data_t *state, control_signal_t *u_signal)
{
  switch (newMode)
      {
      case Control_mode_rate:
        /*-----------------------------Rate mode-----------------------
         * Only change to rate mode if there is a valid state.
         */
        if(!((state->state_valid_bf & (1 << pitch_rate_bf)) && (state->state_valid_bf & (1 << roll_rate_bf))))
        {
          return;
        }
        Pid_SetMode(param->RatePitch, pid_on, u_signal->control_signal[u_pitch], state->state_bf[pitch_rate_bf]);
        Pid_SetMode(param->RateRoll, pid_on, u_signal->control_signal[u_roll], state->state_bf[roll_rate_bf]);
        //Pid_SetMode(param->RateYaw, pid_on, 0, state->state_bf[pitch_rate_bf]);
        Pid_SetMode(param->AttitudePitch, pid_off, 0, 0);
        Pid_SetMode(param->AttitudeRoll, pid_off, 0, 0);
        //Pid_SetMode(param->AttitudeYaw, pid_off, 0, 0);
        break;
      case Control_mode_attitude:
        /*-----------------------------Attitude mode-------------------
         * Only change to attitude mode if there is a valid state.
         */
        if(!((state->state_valid_bf & (1 << pitch_bf)) && (state->state_valid_bf & (1 << pitch_bf))))
        {
          return;
        }
        Pid_SetMode(param->AttitudePitch, pid_on, u_signal->control_signal[u_pitch], state->state_bf[pitch_bf]);
        Pid_SetMode(param->AttitudeRoll, pid_on, u_signal->control_signal[u_roll], state->state_bf[pitch_bf]);
        //Pid_SetMode(param->AttitudeYaw, pid_on, 0, 0);
        Pid_SetMode(param->RatePitch, pid_off, 0, 0);
        Pid_SetMode(param->RateRoll, pid_off, 0, 0);
        //Pid_SetMode(param->RateYaw, pid_off, 0, 0);
        break;
      case Control_mode_not_available:
        /*----------------------------Mode not available------------
         * Something is very wrong, we have an unknown state.
         */
        break;
      default:
        break;
      }
}


void Ctrl_Off(CtrlObj_t * param)
{
  switch (Ctrl_GetCurrentMode(param->ModeHandler))
      {
      case Control_mode_rate:
        /*-----------------------------Rate mode-----------------------
         *
         */
        Pid_SetMode(param->RatePitch, pid_off, 0, 0);
        Pid_SetMode(param->RateRoll, pid_off, 0, 0);
        Pid_SetMode(param->RateYaw, pid_off, 0, 0);
        break;
      case Control_mode_attitude:
        /*-----------------------------Attitude mode-------------------
         *
         */
        Pid_SetMode(param->AttitudePitch, pid_off, 0, 0);
        Pid_SetMode(param->AttitudeRoll, pid_off, 0, 0);
        Pid_SetMode(param->AttitudeYaw, pid_off, 0, 0);
        break;
      case Control_mode_not_available:
        /*----------------------------Mode not available------------
         * Something is very wrong, we have an unknown state.
         */
        break;
      default:
        break;
      }
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

