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

#include "Scott-E/inc/Scott-E_MainTask.h"

#include "Components/FlightModeHandler/inc/flight_mode_handler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "QuadFC/QuadFC_Gpio.h"
#include "QuadFC/QuadFC_Serial.h"

#include "Components/AppLog/inc/AppLog.h"

/* Modules */
#include "EventHandler/inc/event_handler.h"
#include "Parameters/inc/parameters.h"
#include "GenericRcSetpointHandler/inc/rc_sp_handler.h"
#include "SpectrumToGenericRC/inc/SpectrumToGenericRC.h"
#include "FlightModeHandler/inc/flight_mode_handler.h"
#include "Log/inc/logHandler.h"
#include "Log/inc/log.h"
#include "SabertoothSerial/inc/SabertoothSerial.h"
#include "Messages/inc/Msg_Param.h"

#include "Messages/inc/Msg_InitExternal.h"

#define LOOP_TIME (CTRL_TIME * 50) // CTRL_TIME is 1ms, run this each 50 ms.

#define E_STOP_PIN (Gpio_Pwm1)
#define RELAY_PIN (Gpio_Pwm2)


const static unsigned int SpTimeoutMs = 500;

static int motorSelect = 0;

typedef struct Scott_E
{
    int32_t drive_m1;
    int32_t drive_m2;
    int32_t tool_1;
    int32_t tool_2;
    int32_t tool_pitch;
    int32_t tool_yaw;
} Scott_E_t;
// Struct only used here to pass parameters to the task.
typedef struct mainTaskParams
{
    genericRC_t *setpoint;
    Scott_E_t motors;
    FlightModeHandler_t *flightModeHandler;
    RcSpHandler_t *setPointHandler;
    spectrumGenericHandler_t *spectrumGenericHandler;
    eventHandler_t *evHandler;
    LogHandler_t *logHandler;
    paramHander_t *paramHandler;
    int32_t SetpointTimeoutCounter;
    Sabertooth_t *SaberToothDrive;
    Sabertooth_t *SaberToothTool;
    Sabertooth_t *SaberToothArticulation;
    uint32_t uartNrSabertooth;
    int32_t eStopRcState;
    int32_t armingSwithchStateRc;

} MaintaskParams_t;

uint8_t Scott_E_InitExternal(eventHandler_t *obj, void *data, moduleMsg_t *msg);

void Scott_E_SpToSaber(MaintaskParams_t *param, FlightMode_t mode);

void Scott_e_Task(void *pvParameters);
/**
 * Puts FC into a fault state, motors are stopped and control system is
 * shut off.
 * @param param
 */
void main_fault(MaintaskParams_t *param);


void Scott_E_Estop();
void Scott_E_ReleaseEstop();
void Scott_E_PowerDriveMotors();
void Scott_E_PowerDownDriveMotors();

/*
 * void create_main_control_task(void)
 */

void Scott_e_CreateTask(eventHandler_t *evHandler, uint32_t uartNrSabertooth)
{
    /*Create the task*/
    Gpio_Init(HEARTBEAT, Gpio_OutputOpenDrain, Gpio_NoPull);
    Gpio_Init(E_STOP_PIN, Gpio_OutputOpenDrain, Gpio_NoPull); // used as e-stop.
    Gpio_Init(RELAY_PIN, Gpio_OutputPullPush, Gpio_NoPull); // Control relay powering main drive motors.
    Scott_E_Estop(); // start in e-stop.
    Scott_E_PowerDriveMotors(); // start in powerON state

    MaintaskParams_t *taskParam = pvPortMalloc(sizeof(MaintaskParams_t));
    taskParam->paramHandler = ParamHandler_CreateObj(10, evHandler, "Scott-E"); // Master handles all communication, we do not want to be master!
    taskParam->setpoint = pvPortMalloc(sizeof(state_data_t));
    taskParam->flightModeHandler = FMode_CreateFmodeHandler(evHandler); // registers event handler for flight mode requests.
    taskParam->setPointHandler = RcSpHandl_Create(evHandler);
    taskParam->spectrumGenericHandler = SatGenericHandler_CreateObj(evHandler);
    taskParam->evHandler = evHandler;
    taskParam->logHandler = LogHandler_CreateObj(20, evHandler, ParamHandler_GetParam(taskParam->paramHandler), "LogHandler");
    taskParam->SaberToothDrive = Sabertooth_Create(128, uartNrSabertooth);
    taskParam->SaberToothTool = Sabertooth_Create(129, uartNrSabertooth);
    taskParam->SaberToothArticulation = Sabertooth_Create(130, uartNrSabertooth);

    taskParam->SetpointTimeoutCounter = 0;
    taskParam->uartNrSabertooth = uartNrSabertooth;
    taskParam->eStopRcState = 0;
    taskParam->armingSwithchStateRc = 0;
    
    Event_RegisterCallback(taskParam->evHandler, Msg_InitExternal_e, Scott_E_InitExternal, taskParam);

    /*Ensure that all mallocs returned valid pointers*/
    if (!taskParam || !taskParam->paramHandler || !taskParam->setpoint || !taskParam->flightModeHandler ||
        !taskParam->setPointHandler || !taskParam->spectrumGenericHandler || !taskParam->evHandler || !taskParam->logHandler ||
        !taskParam->SaberToothDrive || !taskParam->SaberToothTool || !taskParam->SaberToothArticulation)

    {
        for (;;)
        {
            // TODO error handling!
        }
    }
    QuadFC_SerialOptions_t opt = {
        38400,
        EightDataBits,
        NoParity,
        OneStopBit,
        NoFlowControl,
        NULL,
        1};
        
    uint8_t rsp = QuadFC_SerialInit(uartNrSabertooth, &opt);
    if (!rsp)
    {
        /*Error - Could not create serial interface.*/

        for (;;)
        {
        }
    }

    // Event_RegisterCallback(taskParam->evHandler, ePeripheralError  ,Led_HandlePeripheralError);

    portBASE_TYPE create_result;
    create_result = xTaskCreate(Scott_e_Task,                 /* The function that implements the task.  */
                                (const char *const)"Scott_e", /* The name of the task. This is not used by the kernel, only aids in debugging*/
                                500,                          /* The stack size for the task*/
                                taskParam,                    /* pass params to task.*/
                                configMAX_PRIORITIES - 1,     /* The priority of the task, never higher than configMAX_PRIORITIES -1*/
                                NULL);                        /* Handle to the task. Not used here and therefore NULL*/

    if (create_result != pdTRUE)
    {
        /*Error - Could not create task.*/
        for (;;)
        {
        }
    }
}

/*
 *Task writing whatever is in the queue to the a serial port. The serial port is
 *passed as an argument to the task.
 */
void Scott_e_Task(void *pvParameters)
{
    MaintaskParams_t *param = (MaintaskParams_t *)(pvParameters);

    /*Initialize modules*/
    FMode_InitFModeHandler(param->flightModeHandler);

    Scott_E_Estop(); // Estop should be enabled
    Scott_E_PowerDownDriveMotors(); // Power down the motor controller.
    // Load saved parameters.
    //moduleMsg_t *msg = Msg_ParamCreate(FC_Param_e, 0, param_load, 0, 0, 0);
    //Event_Send(param->evHandler, msg);

    /*Utility variables*/
    TickType_t arming_counter = 0;
    uint32_t heartbeat_counter = 0;

    Log_t *drive_m1 = Log_CreateObj(0, variable_type_fp_16_16, &param->motors.drive_m1, NULL, param->logHandler, "drive_m1");
    Log_t *drive_m2 = Log_CreateObj(0, variable_type_fp_16_16, &param->motors.drive_m2, NULL, param->logHandler, "drive_m2");
    Log_t *ToolPitch = Log_CreateObj(0, variable_type_fp_16_16, &param->motors.tool_pitch, NULL, param->logHandler, "ToolPitch");
    Log_t *ToolYaw = Log_CreateObj(0, variable_type_fp_16_16, &param->motors.tool_yaw, NULL, param->logHandler, "ToolYaw");
    Log_t *Tool_1 = Log_CreateObj(0, variable_type_fp_16_16, &param->motors.tool_1, NULL, param->logHandler, "Tool_1");
    Log_t *Tool_2 = Log_CreateObj(0, variable_type_fp_16_16, &param->motors.tool_2, NULL, param->logHandler, "Tool_2");
    Log_t *eStopRc = Log_CreateObj(0, variable_type_fp_16_16, &param->eStopRcState, NULL, param->logHandler, "eStopRC");
    Log_t *armingSw = Log_CreateObj(0, variable_type_fp_16_16, &param->armingSwithchStateRc, NULL, param->logHandler, "armingSw");
    Log_t *motorSelectLog = Log_CreateObj(0, variable_type_fp_16_16, &motorSelect, NULL, param->logHandler, "motorSelect");

    /*The main control loop*/

    unsigned portBASE_TYPE xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {

        /*
         * Main state machine. Controls the state of the flight controller.
         *
         * Get current setpoint. If there is no active setpoint start a counter
         * that will disarm the copter when it expires. If a valid setpoint
         * is received before the counter expires, then it is stopped.
         * */

        FlightMode_t mode = FMode_GetCurrent(param->flightModeHandler);


        if (RcSpHandl_GetSetpoint(param->setPointHandler, param->setpoint, 0)) // We have an active setpoint!
        {
            param->SetpointTimeoutCounter = 0;
            Scott_E_SpToSaber(param, mode); // take care of the setpoint!

        }
        else // we do not have an active setpoint!
        {
            param->SetpointTimeoutCounter++;
        }
        if (param->SetpointTimeoutCounter >= SpTimeoutMs / LOOP_TIME)
        {
            if(FMode_GetCurrent(param->flightModeHandler) == fmode_arming || 
               FMode_GetCurrent(param->flightModeHandler) == fmode_armed)
            {
                LOG_ENTRY(param->evHandler, "Main: Error - Setpoint timeout!");
                main_fault(param);
            }

        }

        switch (FMode_GetCurrent(param->flightModeHandler))
        {
        case fmode_init:
            /*---------------------------------------Initialize mode------------------------
             * Initialize all modules then set the fc to disarmed state.
             */
            Scott_E_PowerDriveMotors(); // Power up the motor controller so that we can configure timeouts etc.
            vTaskDelay(700/LOOP_TIME); // wait so that the motor controller has time to start up.

            Sabertooth_SetTimeout(param->SaberToothDrive, 3);
            Sabertooth_SetTimeout(param->SaberToothArticulation, 4);
            Sabertooth_SetTimeout(param->SaberToothTool, 4);
            
            xLastWakeTime = xTaskGetTickCount();

            if (pdTRUE != FMode_ChangeFMode(param->flightModeHandler, fmode_disarming))
            {
                LOG_ENTRY(param->evHandler, "Main: Error - Failed to change flight mode to disarming!");
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

            if (arming_counter >= (1000 / LOOP_TIME)) // Be in arming state for 1s.
            {
                Scott_E_ReleaseEstop(); // release e-stop
                arming_counter = 0;
                if (!FMode_ChangeFMode(param->flightModeHandler, fmode_armed))
                {
                    LOG_ENTRY(param->evHandler, "Main: Error - Failed to change flight mode to armed!");
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
             */
            if(( motorSelect > -DOUBLE_TO_FIXED(0.3, MAX16f) ) && ( DOUBLE_TO_FIXED(0.3, MAX16f) >  motorSelect ))
            {
                Sabertooth_DriveM1(param->SaberToothDrive, param->motors.drive_m1);
                Sabertooth_DriveM2(param->SaberToothDrive, param->motors.drive_m2);
            }
            else if( DOUBLE_TO_FIXED(0.3, MAX16f) > motorSelect )
            {
                param->motors.drive_m2 = 0;
                Sabertooth_DriveM1(param->SaberToothDrive, param->motors.drive_m1);
            }
            else
            {
                param->motors.drive_m1 = 0;
                Sabertooth_DriveM2(param->SaberToothDrive, param->motors.drive_m2);
            }
            Sabertooth_DriveM1(param->SaberToothArticulation, param->motors.tool_pitch);
            Sabertooth_DriveM2(param->SaberToothArticulation, param->motors.tool_yaw);
            Sabertooth_DriveM1(param->SaberToothTool, param->motors.tool_1);
            Sabertooth_DriveM2(param->SaberToothTool, param->motors.tool_2);



            Log_Report(drive_m1);
            Log_Report(drive_m2);
            Log_Report(ToolPitch);
            Log_Report(ToolYaw);
            Log_Report(Tool_1);
            Log_Report(Tool_2);
            Log_Report(armingSw);
            Log_Report(eStopRc);
            Log_Report(motorSelectLog);

            break;
        case fmode_disarming:
            /*---------------------------------------Disarming mode--------------------------
             * Transitional state. Only executed once.
             */
            Sabertooth_DriveM1(param->SaberToothDrive, 0);
            Sabertooth_DriveM2(param->SaberToothDrive, 0);
            Sabertooth_DriveM1(param->SaberToothArticulation, 0);
            Sabertooth_DriveM2(param->SaberToothArticulation, 0);
            Sabertooth_DriveM1(param->SaberToothTool, 0);
            Sabertooth_DriveM2(param->SaberToothTool, 0);
            Scott_E_Estop(); // pull e-stop when disarming. This should ensure all motors are stopped. Slightly redundant.
            if (!FMode_ChangeFMode(param->flightModeHandler, fmode_disarmed))
            {
                LOG_ENTRY(param->evHandler, "Main: Error - Failed to change flight mode to disarmed.");
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
        case fmode_exitFault:
            /*-----------------------------------------fault mode---------------------------
             *  Something has gone wrong and caused the FC to go into fault mode. Nothing
             *  happens in fault mode. Only allowed transition is to disarmed mode.
             *
             *  TODO save reason for fault mode.
             */
            Scott_E_PowerDriveMotors();
            if(pdTRUE != FMode_ChangeFMode(param->flightModeHandler, fmode_disarming))
            {
                LOG_ENTRY(param->evHandler, "Main: Error - Failed to change flight mode to disarming!");
                main_fault(param);
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

        // Process incoming events.
        while (Event_Receive(param->evHandler, 0) == 1)
        {
        }

        /*-------------------Heartbeat----------------------
         * Toggle an led to indicate that the Controller is operational.
         */
        heartbeat_counter++;
        if (heartbeat_counter >= 10)
        {
            Gpio_TogglePin(HEARTBEAT);
            heartbeat_counter = 0;
        }


        vTaskDelayUntil(&xLastWakeTime, LOOP_TIME);


    }
    /* The task should never escape the for-loop and therefore never return.*/
}

void main_fault(MaintaskParams_t *param)
{
    // MotorCtrl_Disable(param->motorControl);
    Sabertooth_DriveM1(param->SaberToothDrive, 0);
    Sabertooth_DriveM2(param->SaberToothDrive, 0);
    Sabertooth_DriveM1(param->SaberToothArticulation, 0);
    Sabertooth_DriveM2(param->SaberToothArticulation, 0);
    Sabertooth_DriveM1(param->SaberToothTool, 0);
    Sabertooth_DriveM2(param->SaberToothTool, 0);

    Scott_E_PowerDownDriveMotors(); // Turn off power to motor controller.
    Scott_E_Estop();
    FMode_Fault(param->flightModeHandler);
}

void Scott_E_Estop()
{
    Gpio_PinReset(E_STOP_PIN);
}

void Scott_E_ReleaseEstop()
{
    Gpio_PinSet(E_STOP_PIN);
}

void Scott_E_PowerDriveMotors()
{
    Gpio_PinSet(RELAY_PIN);
}

void Scott_E_PowerDownDriveMotors()
{
    Gpio_PinReset(RELAY_PIN);
}

uint8_t Scott_E_InitExternal(eventHandler_t *obj, void *data, moduleMsg_t *msg)
{
    if (!obj || !data || !msg)
    {
        return 0;
    }

    MaintaskParams_t* taskParam = (MaintaskParams_t*)data;

    QuadFC_SerialOptions_t opt = {
        9600,
        EightDataBits,
        NoParity,
        OneStopBit,
        NoFlowControl,
        NULL,
        1};
    // set baud to sabertooth default
    QuadFC_SerialDeInit(taskParam->uartNrSabertooth);
    uint8_t rsp = QuadFC_SerialInit(taskParam->uartNrSabertooth, &opt);
    if (!rsp)
    {
        /*Error - Could not create serial interface.*/
        LOG_ENTRY(obj, "Not able to initialize serial port in InitExternal call. Unknown status of external units.")
        return 0;
    }

    Sabertooth_SetTimeout(taskParam->SaberToothDrive, 3);
    Sabertooth_SetTimeout(taskParam->SaberToothArticulation, 3);
    Sabertooth_SetTimeout(taskParam->SaberToothTool, 3);


    // update the sabertooth baud on the connected units.
    Sabertooth_SetBaud(taskParam->SaberToothArticulation, 4);
    Sabertooth_SetBaud(taskParam->SaberToothDrive, 4);
    Sabertooth_SetBaud(taskParam->SaberToothTool, 4);

    QuadFC_SerialOptions_t opt1 = {
        38400,
        EightDataBits,
        NoParity,
        OneStopBit,
        NoFlowControl,
        NULL,
        1};
    // set the baud to the newly updated value. This should be persistant. 
    QuadFC_SerialDeInit(taskParam->uartNrSabertooth);
    uint8_t rsp1 = QuadFC_SerialInit(taskParam->uartNrSabertooth, &opt1);
    if (!rsp1)
    {
        /*Error - Could not create serial interface.*/
        LOG_ENTRY(obj, "Not able to initialize serial port in InitExternal call. Unknown status of external units.")
        return 0;
    }


    return 1;
}

void Scott_E_SpToSaber(MaintaskParams_t *param, FlightMode_t mode)
{
    int32_t drive = param->setpoint->channel[2]; //drive
    int32_t turn = param->setpoint->channel[1]; //turn  

    param->motors.tool_pitch = param->setpoint->channel[0]; //throttle
    param->motors.drive_m1   = drive + turn;
    param->motors.drive_m2   = drive - turn;
    param->motors.tool_yaw   = param->setpoint->channel[3]; //tool 
    param->armingSwithchStateRc = param->setpoint->channel[4]; //gear

    //param->motors.tool_2     = param->setpoint->channel[5]; //Tool2 
    motorSelect = param->setpoint->channel[5]; //Tool2 

    param->motors.tool_1     = param->setpoint->channel[6]; //tool 1 
    
    param->eStopRcState      = param->setpoint->channel[7]; //aux 3
    

    if(mode == fmode_init || mode == fmode_not_available)
    {
        return; // Dont do anyting during init or fault condition.
    }
    if(param->eStopRcState < 0)/*Estop not requested*/
    {
        if (param->armingSwithchStateRc < 0) /*Disarm requested*/
        {
            if(mode == fmode_fault)
            {
                if(pdTRUE != FMode_ChangeFMode(param->flightModeHandler, fmode_exitFault))
                {
                    LOG_ENTRY(param->evHandler, "Main: Error - Failed to change flight mode to fmode_exitFault!");
                    main_fault(param);
                }
            }
            else if(mode != fmode_disarming && mode != fmode_disarmed)
            {
                if(pdTRUE != FMode_ChangeFMode(param->flightModeHandler, fmode_disarming))
                {
                    LOG_ENTRY(param->evHandler, "Main: Error - Failed to change flight mode to fmode_disarming!");
                    main_fault(param);
                }
            }
        }
        else/*Arming request*/
        {
            if(mode != fmode_arming && mode != fmode_armed && mode != fmode_init)
            {
                if(pdTRUE != FMode_ChangeFMode(param->flightModeHandler, fmode_arming))
                {
                    LOG_ENTRY(param->evHandler, "Main: Error - Failed to change flight mode to fmode_arming!");
                    main_fault(param);
                }
            }
        }
    }
    else/*Estop requested*/
    {
        if(mode != fmode_fault)
        {
            LOG_ENTRY(param->evHandler, "Main: Error - Estop requested!");
            main_fault(param);
        }
    }   
}
