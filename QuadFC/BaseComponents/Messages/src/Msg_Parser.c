/*
 * Msg_Parser.c
 *
 * Copyright (C) 2019 Martin Lundh
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
#include "../inc/Msg_Parser.h"
#include "../../Messages/inc/Msg_Param.h"
#include "../../Messages/inc/Msg_Log.h"
#include "../../Messages/inc/Msg_AppLog.h"
#include "../../Messages/inc/Msg_Debug.h"
#include "../../Messages/inc/Msg_TestMem.h"
#include "../../Messages/inc/Msg_TestMemReg.h"
#include "../../Messages/inc/Msg_Transmission.h"
#include "../../Messages/inc/Msg_TestTransmission.h"
#include "../../Messages/inc/Msg_FlightMode.h"
#include "../../Messages/inc/Msg_CtrlMode.h"
#include "../../Messages/inc/Msg_FcFault.h"
#include "../../Messages/inc/Msg_Error.h"
#include "../../Messages/inc/Msg_BindRc.h"
#include "../../Messages/inc/Msg_FlightModeReq.h"
#include "../../Messages/inc/Msg_CtrlModeReq.h"
#include "../../Messages/inc/Msg_NewSetpoint.h"
#include "../../Messages/inc/Msg_RegisterHandler.h"
#include "../../Messages/inc/Msg_Subscriptions.h"
#include "../../Messages/inc/Msg_FcLog.h"
#include "../../Messages/inc/Msg_LogNameReq.h"
#include "../../Messages/inc/LogStop.h"
#include "../../Messages/inc/Msg_CtrlSig.h"
#include "../../Messages/inc/Msg_NewState.h"
#include "../../Messages/inc/Msg_HasParam.h"
#include "../../Messages/inc/Msg_ParamFc.h"
#include "../../Messages/inc/Msg_SpectrumData.h"
#include "../../Messages/inc/Msg_ValidSp.h"
#include "../../Messages/inc/Msg_ChangeComPort.h"


moduleMsg_t* Msg_Parse(uint8_t* buffer, uint32_t bufferLength)
{
    moduleMsg_t* msg = Msg_Create(0,0);
    Msg_DeSerialize(msg, buffer, bufferLength);
    messageTypes_t type = Msg_GetType(msg);
    Msg_Delete(&msg);
    
    moduleMsg_t* msgResult = NULL;

    switch(type)
    {
    
    case Msg_Param_e:
    {
        msgResult = Msg_ParamDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_Log_e:
    {
        msgResult = Msg_LogDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_AppLog_e:
    {
        msgResult = Msg_AppLogDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_Debug_e:
    {
        msgResult = Msg_DebugDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_TestMem_e:
    {
        msgResult = Msg_TestMemDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_TestMemReg_e:
    {
        msgResult = Msg_TestMemRegDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_Transmission_e:
    {
        msgResult = Msg_TransmissionDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_TestTransmission_e:
    {
        msgResult = Msg_TestTransmissionDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_FlightMode_e:
    {
        msgResult = Msg_FlightModeDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_CtrlMode_e:
    {
        msgResult = Msg_CtrlModeDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_FcFault_e:
    {
        msgResult = Msg_FcFaultDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_Error_e:
    {
        msgResult = Msg_ErrorDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_BindRc_e:
    {
        msgResult = Msg_BindRcDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_FlightModeReq_e:
    {
        msgResult = Msg_FlightModeReqDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_CtrlModeReq_e:
    {
        msgResult = Msg_CtrlModeReqDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_NewSetpoint_e:
    {
        msgResult = Msg_NewSetpointDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_RegisterHandler_e:
    {
        msgResult = Msg_RegisterHandlerDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_Subscriptions_e:
    {
        msgResult = Msg_SubscriptionsDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_FcLog_e:
    {
        msgResult = Msg_FcLogDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_LogNameReq_e:
    {
        msgResult = Msg_LogNameReqDeserialize(buffer, bufferLength);
        break;
    }
    
    case LogStop_e:
    {
        msgResult = LogStopDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_CtrlSig_e:
    {
        msgResult = Msg_CtrlSigDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_NewState_e:
    {
        msgResult = Msg_NewStateDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_HasParam_e:
    {
        msgResult = Msg_HasParamDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_ParamFc_e:
    {
        msgResult = Msg_ParamFcDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_SpectrumData_e:
    {
        msgResult = Msg_SpectrumDataDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_ValidSp_e:
    {
        msgResult = Msg_ValidSpDeserialize(buffer, bufferLength);
        break;
    }
    
    case Msg_ChangeComPort_e:
    {
        msgResult = Msg_ChangeComPortDeserialize(buffer, bufferLength);
        break;
    }
    
    default:
    {
    }
    }

    return msgResult;
}

uint8_t* Msg_Serializer(moduleMsg_t* msg, uint8_t* buffer, uint32_t bufferLength)
{
    messageTypes_t type = Msg_GetType(msg);
    uint8_t* result = NULL;

    switch(type)
    {
        case Msg_Param_e:
    {
        result = Msg_ParamSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_Log_e:
    {
        result = Msg_LogSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_AppLog_e:
    {
        result = Msg_AppLogSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_Debug_e:
    {
        result = Msg_DebugSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_TestMem_e:
    {
        result = Msg_TestMemSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_TestMemReg_e:
    {
        result = Msg_TestMemRegSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_Transmission_e:
    {
        result = Msg_TransmissionSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_TestTransmission_e:
    {
        result = Msg_TestTransmissionSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_FlightMode_e:
    {
        result = Msg_FlightModeSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_CtrlMode_e:
    {
        result = Msg_CtrlModeSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_FcFault_e:
    {
        result = Msg_FcFaultSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_Error_e:
    {
        result = Msg_ErrorSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_BindRc_e:
    {
        result = Msg_BindRcSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_FlightModeReq_e:
    {
        result = Msg_FlightModeReqSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_CtrlModeReq_e:
    {
        result = Msg_CtrlModeReqSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_NewSetpoint_e:
    {
        result = Msg_NewSetpointSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_RegisterHandler_e:
    {
        result = Msg_RegisterHandlerSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_Subscriptions_e:
    {
        result = Msg_SubscriptionsSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_FcLog_e:
    {
        result = Msg_FcLogSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_LogNameReq_e:
    {
        result = Msg_LogNameReqSerialize(msg, buffer, bufferLength);
        break;
    }
        case LogStop_e:
    {
        result = LogStopSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_CtrlSig_e:
    {
        result = Msg_CtrlSigSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_NewState_e:
    {
        result = Msg_NewStateSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_HasParam_e:
    {
        result = Msg_HasParamSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_ParamFc_e:
    {
        result = Msg_ParamFcSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_SpectrumData_e:
    {
        result = Msg_SpectrumDataSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_ValidSp_e:
    {
        result = Msg_ValidSpSerialize(msg, buffer, bufferLength);
        break;
    }
        case Msg_ChangeComPort_e:
    {
        result = Msg_ChangeComPortSerialize(msg, buffer, bufferLength);
        break;
    }
    
    default:
    {
    }
    }

    return result;
}
