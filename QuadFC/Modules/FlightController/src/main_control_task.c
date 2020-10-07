/*
 * main_control_task.c
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

/* QuadFC - A quadcopter flight controller.
 * 
 *
 * This is the main task in the QuadFC.
 */

#include "FlightController/inc/main_control_task.h"

#include "FlightModeHandler/inc/flight_mode_handler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include "HAL/QuadFC/QuadFC_Gpio.h"

#include "StateEstimator/inc/signal_processing.h"

#include "Components/AppLog/inc/AppLog.h"

/* Modules */
#include "FlightController/inc/control_system.h"
#include "EventHandler/inc/event_handler.h"
#include "StateEstimator/inc/state_estimator.h"
#include "QuadFC/QuadFC_MotorControl.h"
#include "Parameters/inc/parameters.h"
#include "SetpointHandler/inc/setpoint_handler.h"
#include "SpectrumSatellite/inc/Satellite_SetpointHandler.h"
#include "FlightController/inc/control_mode_handler.h"
#include "FlightModeHandler/inc/flight_mode_handler.h"
#include "Log/inc/logHandler.h"
#include "Log/inc/log.h"


//TODO remove
#include "Parameters/inc/parameters.h"

const static unsigned int SpTimeoutMs = 500;

// Struct only used here to pass parameters to the task.
typedef struct mainTaskParams
{
    state_data_t *state;
    state_data_t *setpoint;
    CtrlObj_t *ctrl;
    control_signal_t *control_signal;
    MotorControlObj *motorControl;
    FlightModeHandler_t* flightModeHandler;
    StateEst_t* stateEst;
    SpHandler_t* setPointHandler;
    spectrumSpHandler_t* spectrumSpHandler;
    CtrlModeHandler_t * CtrlModeHandler;
    eventHandler_t* evHandler;
    LogHandler_t* logHandler;
    paramHander_t* paramHandler;
    int32_t SetpointTimeoutCounter;
}MaintaskParams_t;

void main_control_task( void *pvParameters );
/**
 * Puts FC into a fault state, motors are stopped and control system is
 * shut off.
 * @param param
 */
void main_fault(MaintaskParams_t *param);

/*
 * void create_main_control_task(void)
 */

void create_main_control_task(eventHandler_t* evHandler)
{
    /*Create the task*/

    MaintaskParams_t * taskParam = pvPortMalloc(sizeof(MaintaskParams_t));
    taskParam->paramHandler = ParamHandler_CreateObj(8,evHandler, "QuadGS", 0); // Master handles all communication, we do not want to be master!
    taskParam->ctrl = Ctrl_Create(ParamHandler_GetParam(taskParam->paramHandler));
    taskParam->setpoint = pvPortMalloc( sizeof(state_data_t) );
    taskParam->state = pvPortMalloc( sizeof(state_data_t) );
    taskParam->control_signal = pvPortMalloc( sizeof(control_signal_t) );
    taskParam->motorControl = MotorCtrl_CreateAndInit(4,ParamHandler_GetParam(taskParam->paramHandler));
    taskParam->flightModeHandler = FMode_CreateFmodeHandler(evHandler); // registers event handler for flight mode requests.
    taskParam->stateEst = StateEst_Create(ParamHandler_GetParam(taskParam->paramHandler));
    taskParam->setPointHandler = SpHandl_Create(evHandler);
    taskParam->spectrumSpHandler = SatSpHandler_CreateObj(evHandler, ParamHandler_GetParam(taskParam->paramHandler));
    taskParam->CtrlModeHandler = Ctrl_CreateModeHandler(evHandler);
    taskParam->evHandler = evHandler;
    taskParam->logHandler = LogHandler_CreateObj(20,evHandler,ParamHandler_GetParam(taskParam->paramHandler),"LogHandler",0);
    taskParam->SetpointTimeoutCounter = 0;
    /*Ensure that all mallocs returned valid pointers*/
    if (   !taskParam                    || !taskParam->ctrl             || !taskParam->setpoint
            || !taskParam->state             || !taskParam->control_signal   // TODO!!|| !taskParam->motorControl
            || !taskParam->flightModeHandler || !taskParam->stateEst         || !taskParam->setPointHandler
            || !taskParam->logHandler        || !taskParam->paramHandler || !taskParam->spectrumSpHandler)
    {
        for ( ;; )
        {
            // TODO error handling!
        }
    }


    // Event_RegisterCallback(taskParam->evHandler, ePeripheralError  ,Led_HandlePeripheralError);

    portBASE_TYPE create_result;
    create_result = xTaskCreate( main_control_task,   /* The function that implements the task.  */
            (const char *const) "Main_Ctrl",              /* The name of the task. This is not used by the kernel, only aids in debugging*/
            500,                                         /* The stack size for the task*/
            taskParam,                                    /* pass params to task.*/
            configMAX_PRIORITIES-1,                       /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
            NULL );                                       /* Handle to the task. Not used here and therefore NULL*/

    if ( create_result != pdTRUE )
    {
        /*Error - Could not create task.*/
        for ( ;; )
        {
        }
    }

}

/*
 *Task writing whatever is in the queue to the a serial port. The serial port is 
 *passed as an argument to the task. 
 */
void main_control_task( void *pvParameters )
{

    MaintaskParams_t * param = (MaintaskParams_t*)(pvParameters);


    /*Initialize modules*/
    FMode_InitFModeHandler(param->flightModeHandler);
    Ctrl_InitModeHandler(param->CtrlModeHandler);

    //This takes some time due to calibration.
    if(!StateEst_init(param->stateEst))
    {
        LOG_ENTRY(FC_SerialIOtx_e, param->evHandler, "Main: Error - Failed to initialize IMU!");
        main_fault(param);
    }
    /*Utility variables*/
    TickType_t arming_counter = 0;
    uint32_t heartbeat_counter = 0;
    /* Loggers for rate mode */
    Log_t* logPitchRate = Log_CreateObj(0,variable_type_fp_16_16,  &param->state->state_bf[pitch_rate_bf],NULL, param->logHandler, "m_PitchRate");
    Log_t* logRollRate = Log_CreateObj(0,variable_type_fp_16_16, &param->state->state_bf[roll_rate_bf],NULL, param->logHandler, "m_RollRate");
    Log_t* logYawRate = Log_CreateObj(0,variable_type_fp_16_16, &param->state->state_bf[yaw_rate_bf],NULL, param->logHandler, "m_YawRate");

    Log_t* logUPitch = Log_CreateObj(0,variable_type_fp_16_16,  &param->control_signal[u_pitch],NULL, param->logHandler, "u_Pitch");
    Log_t* logURoll = Log_CreateObj(0,variable_type_fp_16_16, &param->control_signal[u_roll],NULL, param->logHandler, "u_Roll");
    Log_t* logUYaw = Log_CreateObj(0,variable_type_fp_16_16, &param->control_signal[u_yaw],NULL, param->logHandler, "u_Yaw");
    Log_t* logUThrust = Log_CreateObj(0,variable_type_fp_16_16, &param->control_signal[u_thrust],NULL, param->logHandler, "u_Throttle");

    Log_t* logSpPitchRate = Log_CreateObj(0,variable_type_fp_16_16,  &param->setpoint->state_bf[pitch_rate_bf],NULL, param->logHandler, "sp_PitchRate");
    Log_t* logSpRollRate = Log_CreateObj(0,variable_type_fp_16_16, &param->setpoint->state_bf[roll_rate_bf],NULL, param->logHandler, "sp_RollRate");
    Log_t* logSpYawRate = Log_CreateObj(0,variable_type_fp_16_16, &param->setpoint->state_bf[yaw_rate_bf],NULL, param->logHandler, "sp_YawRate");
    Log_t* logSpThrustRate = Log_CreateObj(0,variable_type_fp_16_16, &param->setpoint->state_bf[thrust_sp],NULL, param->logHandler, "sp_Throttle");

    /*The main control loop*/

    unsigned portBASE_TYPE xLastWakeTime = xTaskGetTickCount();
    for ( ;; )
    {
        /*
         * Main state machine. Controls the state of the flight controller.
         */
        switch (FMode_GetCurrent(param->flightModeHandler))
        {
        case fmode_init:
            /*---------------------------------------Initialize mode------------------------
             * Initialize all modules then set the fc to disarmed state.
             */

            // StateEst_init Might take time, reset the last wake time to avoid errors.
            xLastWakeTime = xTaskGetTickCount();

            if(pdTRUE != FMode_ChangeFMode(param->flightModeHandler, fmode_disarming))
            {
                LOG_ENTRY(FC_SerialIOtx_e, param->evHandler, "Main: Error - Failed to change flight mode to disarming!");
                main_fault(param);
            }

            break;
        case fmode_disarmed:
            /*---------------------------------------Disarmed mode--------------------------
             * Nothing will happen in disarmed mode. Allowed to change to configure mode,
             * arming mode and fault mode.
             */
            break;
        case fmode_config:
            /*---------------------------------------Configure mode-------------------------
             * Configure mode is used to update parameters used in the control of the copter.
             * The copter has to be disarmed before entering configure mode. To enter a specific
             * command has to be sent over USART. Configure mode can be exited by either
             * restarting the FC or by sending a specific command over the USART communication
             * channel.
             */
            break;
        case fmode_arming:
            /*-----------------------------------------arming mode--------------------------
             * Before transition into armed mode the fc always passes through arming state to
             * ensure that all systems are available and ready. Use the counter method rather
             * than sleeping to allow external communication.
             */
            MotorCtrl_Enable( param->motorControl );
            Ctrl_On(param->ctrl);

            if ( arming_counter >= ( 1000 / CTRL_TIME ) ) // Be in arming state for 1s.
            {
                arming_counter = 0;
                if( !FMode_ChangeFMode(param->flightModeHandler, fmode_armed) )
                {
                    LOG_ENTRY(FC_SerialIOtx_e, param->evHandler, "Main: Error - Failed to change flight mode to armed!");
                    main_fault(param);
                }
                
            }
            else
            {
                arming_counter++;
            }
            break;
        case fmode_armed:
            /*-----------------------------------------armed mode---------------------------
             * The FC is armed and operational. The main control loop only handles
             * the Control system.
             *
             * Get current setpoint. If there is no active setpoint start a counter
             * that will disarm the copter when it expires. If a valid setpoint
             * is received before the counter expires, then it is stopped.
             * */
            if(SpHandl_GetSetpoint(param->setPointHandler, param->setpoint, 0 )) // We have an active setpoint!
            {
                param->SetpointTimeoutCounter = 0;
            }
            else // we do not have an active setpoint!
            {
                param->SetpointTimeoutCounter++;
            }

            if(param->SetpointTimeoutCounter >= SpTimeoutMs/CTRL_TIME)
            {
                LOG_ENTRY(FC_SerialIOtx_e, param->evHandler, "Main: Error - Setpoint timeout!");
                main_fault(param);
            }

            // Get the estimated physical state of the copter.
            if(!StateEst_getState(param->stateEst, param->state, Ctrl_GetCurrentMode(param->CtrlModeHandler)))
            {
                LOG_ENTRY(FC_SerialIOtx_e, param->evHandler, "Main: Error - Not able to get the current state.");
                main_fault(param);
            }
            //Execute the control, allocate the control signals to the different
            // motors, and then update the motor setpoint.
            if(!Ctrl_Execute(param->ctrl, param->state, param->setpoint, param->control_signal, Ctrl_GetCurrentMode(param->CtrlModeHandler)))
            {
                LOG_ENTRY(FC_SerialIOtx_e, param->evHandler, "Main: Error - Unable to execute the control loop.");
                main_fault(param);
            }
            Ctrl_Allocate(param->control_signal, param->motorControl->motorSetpoint);
            MotorCtrl_UpdateSetpoint( param->motorControl);


            Log_Report(logPitchRate);
            Log_Report(logRollRate);
            Log_Report(logYawRate);

            Log_Report(logUPitch);
            Log_Report(logURoll);
            Log_Report(logUYaw);
            Log_Report(logUThrust);

            Log_Report(logSpPitchRate);
            Log_Report(logSpRollRate);
            Log_Report(logSpYawRate);
            Log_Report(logSpThrustRate);

            break;
        case fmode_disarming:
            /*---------------------------------------Disarming mode--------------------------
             * Transitional state. Only executed once.
             */
            MotorCtrl_Disable(param->motorControl);
            Ctrl_Off(param->ctrl);
            if( !FMode_ChangeFMode(param->flightModeHandler, fmode_disarmed) )
            {
                LOG_ENTRY(FC_SerialIOtx_e, param->evHandler, "Main: Error - Failed to change flight mode to disarmed.");
                main_fault(param);
            }

            break;
        case fmode_fault:
            /*-----------------------------------------fault mode---------------------------
             *  Something has gone wrong and caused the FC to go into fault mode. Nothing
             *  happens in fault mode. Only allowed transition is to disarmed mode.
             *
             *  TODO save reason for fault mode.
             */
        {
            static int i = 0;
            i++;
        }
        break;
        case fmode_not_available:
            /*-----------------------------------------state not available mode----------------
             * Something is very wrong if the FC is in state_not_available.
             */
            break;
        default:
            break;
        }

        //Process incoming events.
        while(Event_Receive(param->evHandler, 0) == 1)
        {}

        vTaskDelayUntil( &xLastWakeTime, CTRL_TIME );

        /*-------------------Heartbeat----------------------
         * Toggle an led to indicate that the FC is operational.
         */
        heartbeat_counter++;
        if ( heartbeat_counter >= 500 )
        {
            //LOG_ENTRY(FC_SerialIOtx_e, param->evHandler, "%s", "Heartbeat!");
            Led_Toggle( ledHeartBeat );
            heartbeat_counter = 0;

        }
    }
    /* The task should never escape the for-loop and therefore never return.*/
}

void main_fault(MaintaskParams_t *param)
{
    MotorCtrl_Disable(param->motorControl);
    FMode_Fault(param->flightModeHandler);
    Ctrl_Off(param->ctrl);
}

