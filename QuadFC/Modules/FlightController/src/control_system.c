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
#include "Components/PidController/inc/pid.h"

struct CtrlObj
{
    pidConfig_t* RatePitch;
    pidConfig_t* RateRoll;
    pidConfig_t* RateYaw;
    pidConfig_t* AttitudePitch;
    pidConfig_t* AttitudeRoll;
    pidConfig_t* AttitudeYaw;
    CtrlMode_t current_mode;
};

uint8_t Ctrl_InitializeRate(CtrlObj_t *obj, param_obj_t* param);
uint8_t Ctrl_InitializeAttitude(CtrlObj_t *obj, param_obj_t* param);



CtrlObj_t* Ctrl_Create(param_obj_t* param)
{
    CtrlObj_t *obj = pvPortMalloc(sizeof(struct CtrlObj));

    obj->current_mode = Control_mode_not_available;
    if(!obj || !Ctrl_InitializeRate(obj, param) || !Ctrl_InitializeAttitude(obj, param))
    {
        return NULL;
    }
    return obj;
}

uint8_t Ctrl_InitializeRate(CtrlObj_t *obj, param_obj_t* param)
{
    /**
     * The constants (Kp, Ki and Kd) does not have a unit, but are expressed as 16.16 fixed point.
     * Output of the controller should lie in [0, 1<<16] where 1<<16 represent 100% control signal.
     */
    int32_t maxOutputInit = MAX_U_SIGNAL;
    int32_t minOutputInit = -MAX_U_SIGNAL;

    obj->RatePitch = Pid_Create(DOUBLE_TO_FIXED(0.4, MAX16f), DOUBLE_TO_FIXED(0.6, MAX16f), DOUBLE_TO_FIXED(0.001, MAX16f), minOutputInit, maxOutputInit, FP_16_16_SHIFT, CTRL_TIME_FP);
    obj->RateRoll  = Pid_Create(DOUBLE_TO_FIXED(0.4, MAX16f), DOUBLE_TO_FIXED(0.6, MAX16f), DOUBLE_TO_FIXED(0.001, MAX16f), minOutputInit, maxOutputInit, FP_16_16_SHIFT, CTRL_TIME_FP);
    obj->RateYaw   = Pid_Create(DOUBLE_TO_FIXED(1.2, MAX16f), DOUBLE_TO_FIXED(0.6, MAX16f), DOUBLE_TO_FIXED(0.001, MAX16f), minOutputInit, maxOutputInit, FP_16_16_SHIFT, CTRL_TIME_FP);

    if(!obj->RatePitch || !obj->RateRoll || !obj->RateYaw)
    {
        return 0;
    }
    param_obj_t * RateRoot = Param_CreateObj(3, variable_type_NoType, readOnly, NULL, "PID_R", param);

    // Enable tuning of the pid parameters.
    param_obj_t * pitchRateObj = Param_CreateObj(5, variable_type_NoType, readOnly, NULL, "Pitch", RateRoot);
    param_obj_t * rollRateObj  = Param_CreateObj(5, variable_type_NoType, readOnly, NULL, "Roll", RateRoot);
    param_obj_t * yawRateObj   = Param_CreateObj(5, variable_type_NoType, readOnly, NULL, "Yaw", RateRoot);

    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->RatePitch->kp, "Kp", pitchRateObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->RatePitch->ki, "Ki", pitchRateObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->RatePitch->kd, "Kd", pitchRateObj);

    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->RateRoll->kp, "Kp", rollRateObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->RateRoll->ki, "Ki", rollRateObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->RateRoll->kd, "Kd", rollRateObj);

    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->RateYaw->kp, "Kp", yawRateObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->RateYaw->ki, "Ki", yawRateObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->RateYaw->kd, "Kd", yawRateObj);

    return 1;
}

uint8_t Ctrl_InitializeAttitude(CtrlObj_t *obj, param_obj_t* param)
{
    /**
     * The constants (Kp, Ki and Kd) does not have a unit, but are expressed as 16.16 fixed point.
     * Output of the controller should lie in [0, 1<<16] where 1<<16 represent 100% control signal.
     */
    int32_t KpInitial  = 8 << 12;
    int32_t KiInitial  = 0 << (FP_16_16_SHIFT-2);
    int32_t KDInitial  = 0 << (FP_16_16_SHIFT-4);
    int32_t maxOutputInit = MAX_U_SIGNAL;
    int32_t minOutputInit = -MAX_U_SIGNAL;

    obj->AttitudePitch = Pid_Create(KpInitial, KiInitial, KDInitial, minOutputInit, maxOutputInit, FP_16_16_SHIFT, CTRL_TIME_FP);
    obj->AttitudeRoll  = Pid_Create(KpInitial, KiInitial, KDInitial, minOutputInit, maxOutputInit, FP_16_16_SHIFT, CTRL_TIME_FP);
    obj->AttitudeYaw   = Pid_Create(KpInitial, KiInitial, KDInitial, minOutputInit, maxOutputInit, FP_16_16_SHIFT, CTRL_TIME_FP);

    if(!obj->AttitudePitch || !obj->AttitudeRoll || !obj->AttitudeYaw)
    {
        return 0;
    }

    param_obj_t * AttitudeRoot = Param_CreateObj(3, variable_type_NoType, readOnly, NULL, "PID_A", param);

    // Enable tuning of the pid parameters.
    param_obj_t * pitchObj = Param_CreateObj(5, variable_type_NoType, readOnly, NULL, "Pitch", AttitudeRoot);
    param_obj_t * rollObj  = Param_CreateObj(5, variable_type_NoType, readOnly, NULL, "Roll", AttitudeRoot);
    param_obj_t * yawObj   = Param_CreateObj(5, variable_type_NoType, readOnly, NULL, "Yaw", AttitudeRoot);

    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->AttitudePitch->kp, "Kp", pitchObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->AttitudePitch->ki, "Ki", pitchObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->AttitudePitch->kd, "Kd", pitchObj);

    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->AttitudeRoll->kp, "Kp", rollObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->AttitudeRoll->ki, "Ki", rollObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->AttitudeRoll->kd, "Kd", rollObj);

    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->AttitudeYaw->kp, "Kp", yawObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->AttitudeYaw->ki, "Ki", yawObj);
    Param_CreateObj(0, variable_type_fp_16_16, readWrite,
            &obj->AttitudeYaw->kd, "Kd", yawObj);

    return 1;
}

uint8_t Ctrl_Execute(CtrlObj_t *obj, state_data_t *state, state_data_t *setpoint
        , control_signal_t *u_signal, CtrlMode_t current_mode)
{
    if(obj->current_mode != current_mode)
    {
        Ctrl_Switch(obj, current_mode, state, u_signal);
    }

    switch (obj->current_mode)
    {
    case Control_mode_rate:
        /*-----------------------------Rate mode-----------------------
         *
         */
        Pid_Compute(obj->RatePitch, state->state_bf[pitch_rate_bf],
                setpoint->state_bf[pitch_rate_bf],
                &(u_signal->control_signal[u_pitch]));

        Pid_Compute(obj->RateRoll, state->state_bf[roll_rate_bf],
                setpoint->state_bf[roll_rate_bf],
                &(u_signal->control_signal[u_roll]));

        Pid_Compute(obj->RateYaw, state->state_bf[yaw_rate_bf],
                setpoint->state_bf[yaw_rate_bf],
                &(u_signal->control_signal[u_yaw]));

        u_signal->control_signal[u_thrust] = setpoint->state_bf[thrust_sp];
        break;
    case Control_mode_attitude:
        /*-----------------------------Attitude mode-------------------
         *
         */
        Pid_Compute(obj->AttitudePitch, state->state_bf[pitch_bf],
                setpoint->state_bf[pitch_bf],
                &(u_signal->control_signal[u_pitch]));

        Pid_Compute(obj->AttitudeRoll, state->state_bf[roll_bf],
                setpoint->state_bf[roll_bf],
                &(u_signal->control_signal[u_roll]));
        // We do not yet have an angle estimate for the yaw angle, so we
        // use rate of yaw.
        Pid_Compute(obj->AttitudeYaw, state->state_bf[yaw_rate_bf],
                setpoint->state_bf[yaw_rate_bf],
                &(u_signal->control_signal[u_yaw]));

        u_signal->control_signal[u_thrust] = setpoint->state_bf[thrust_sp];
        break;
    case Control_mode_not_available:
        /*----------------------------Mode not available------------
         * Something is very wrong, we have an unknown state.
         *
         */
        return 0;
        break;
    default:
        return 0;
        break;
    }
    return 1;

}

void Ctrl_On(CtrlObj_t * obj)
{
    switch (obj->current_mode)
    {
    case Control_mode_rate:
        /*-----------------------------Rate mode-----------------------
         *
         */
        Pid_SetMode(obj->RatePitch, pid_on, 0, 0);
        Pid_SetMode(obj->RateRoll, pid_on, 0, 0);
        Pid_SetMode(obj->RateYaw, pid_on, 0, 0);
        break;
    case Control_mode_attitude:
        /*-----------------------------Attitude mode-------------------
         *
         */
        Pid_SetMode(obj->AttitudePitch, pid_on, 0, 0);
        Pid_SetMode(obj->AttitudeRoll, pid_on, 0, 0);
        Pid_SetMode(obj->AttitudeYaw, pid_on, 0, 0);
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

uint8_t Ctrl_Switch(CtrlObj_t * param, CtrlMode_t newMode, state_data_t *state
        , control_signal_t *u_signal)
{

    switch (newMode)
    {
    case Control_mode_rate:
        /*-----------------------------Rate mode-----------------------
         * Only change to rate mode if there is a valid state.
         */
        if(!((state->state_valid_bf & (1 << pitch_rate_bf)) && (state->state_valid_bf & (1 << roll_rate_bf))))
        {
            return 0;
        }
        Pid_SetMode(param->RatePitch, pid_on, u_signal->control_signal[u_pitch], state->state_bf[pitch_rate_bf]);
        Pid_SetMode(param->RateRoll, pid_on, u_signal->control_signal[u_roll], state->state_bf[roll_rate_bf]);
        Pid_SetMode(param->RateYaw, pid_on, u_signal->control_signal[u_yaw], state->state_bf[pitch_rate_bf]);
        Pid_SetMode(param->AttitudePitch, pid_off, 0, 0);
        Pid_SetMode(param->AttitudeRoll, pid_off, 0, 0);
        Pid_SetMode(param->AttitudeYaw, pid_off, 0, 0);
        break;
    case Control_mode_attitude:
        /*-----------------------------Attitude mode-------------------
         * Only change to attitude mode if there is a valid state.
         */
        if(!((state->state_valid_bf & (1 << pitch_bf)) && (state->state_valid_bf & (1 << pitch_bf))))
        {
            return 0;
        }
        Pid_SetMode(param->AttitudePitch, pid_on, u_signal->control_signal[u_pitch], state->state_bf[pitch_bf]);
        Pid_SetMode(param->AttitudeRoll, pid_on, u_signal->control_signal[u_roll], state->state_bf[pitch_bf]);
        Pid_SetMode(param->AttitudeYaw, pid_on, u_signal->control_signal[u_yaw], state->state_bf[pitch_rate_bf]);
        Pid_SetMode(param->RatePitch, pid_off, 0, 0);
        Pid_SetMode(param->RateRoll, pid_off, 0, 0);
        Pid_SetMode(param->RateYaw, pid_off, 0, 0);
        break;
    case Control_mode_not_available:
        /*----------------------------Mode not available------------
         * Something is very wrong, we have an unknown state.
         *
         */
        return 0;
        break;
    default:
        break;
    }
    param->current_mode = newMode;
    return 1;
}


void Ctrl_Off(CtrlObj_t * obj)
{
    switch (obj->current_mode)
    {
    case Control_mode_rate:
        /*-----------------------------Rate mode-----------------------
         *
         */
        Pid_SetMode(obj->RatePitch, pid_off, 0, 0);
        Pid_SetMode(obj->RateRoll, pid_off, 0, 0);
        Pid_SetMode(obj->RateYaw, pid_off, 0, 0);
        break;
    case Control_mode_attitude:
        /*-----------------------------Attitude mode-------------------
         *
         */
        Pid_SetMode(obj->AttitudePitch, pid_off, 0, 0);
        Pid_SetMode(obj->AttitudeRoll, pid_off, 0, 0);
        Pid_SetMode(obj->AttitudeYaw, pid_off, 0, 0);
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

void Ctrl_Allocate( control_signal_t *ctrl_signal, uint32_t motor_setpoint[], uint32_t nrMotors)
{
    if(nrMotors < 4)
    {
        for (size_t i = 0; i < nrMotors; i++)
        {
            motor_setpoint[i] = 0; // error case. Turn off motors!
        }
        
        return;
    }
    int32_t tmpSetpoint[4] = {0};

    // Right hand coordinate system with X forward, Y to the left and Z up. Right hand rule for rotations as well.

    // Motor numbering is index + 1.

    tmpSetpoint[0] = ((int32_t) ( ctrl_signal->control_signal[u_thrust] -  ctrl_signal->control_signal[u_roll] / 4 - ctrl_signal->control_signal[u_pitch] / 4 - ctrl_signal->control_signal[u_yaw] / 4));
    tmpSetpoint[1] = ((int32_t) ( ctrl_signal->control_signal[u_thrust] -  ctrl_signal->control_signal[u_roll] / 4 + ctrl_signal->control_signal[u_pitch] / 4 + ctrl_signal->control_signal[u_yaw] / 4));
    tmpSetpoint[2] = ((int32_t) ( ctrl_signal->control_signal[u_thrust] +  ctrl_signal->control_signal[u_roll] / 4 + ctrl_signal->control_signal[u_pitch] / 4 - ctrl_signal->control_signal[u_yaw] / 4));
    tmpSetpoint[3] = ((int32_t) ( ctrl_signal->control_signal[u_thrust] +  ctrl_signal->control_signal[u_roll] / 4 - ctrl_signal->control_signal[u_pitch] / 4 + ctrl_signal->control_signal[u_yaw] / 4));

    for (size_t i = 0; i < nrMotors; i++ )
    {
        if ( tmpSetpoint[i] < 0 )
        {
            tmpSetpoint[i] = 0;
        }
        if(tmpSetpoint[i] > MAX_U_SIGNAL)
        {
            tmpSetpoint[i] = MAX_U_SIGNAL;
        }
    }
    for (size_t i = 0; i < 4; i++)
    {
        motor_setpoint[i] = tmpSetpoint[i];
    }
    
}

