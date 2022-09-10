/*
 * Parser.cpp
 *
 *  Copyright (C) 2017 Martin Lundh
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

#include "Parser.h"

#include "messageTypes.h"
#include "Msg_Stop.h"
#include "Msg_RegisterName.h"
#include "Msg_Display.h"
#include "Msg_GsAppLog.h"
#include "Msg_GsHasLog.h"
#include "Msg_GsLogLevel.h"
#include "Msg_Test.h"
#include "Msg_FindParam.h"
#include "Msg_GetUiCommands.h"
#include "Msg_FireUiCommand.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"
#include "Msg_Param.h"
#include "Msg_Log.h"
#include "Msg_AppLog.h"
#include "Msg_Debug.h"
#include "Msg_TestMem.h"
#include "Msg_TestMemReg.h"
#include "Msg_Transmission.h"
#include "Msg_TestTransmission.h"
#include "Msg_FlightMode.h"
#include "Msg_CtrlMode.h"
#include "Msg_FcFault.h"
#include "Msg_Error.h"
#include "Msg_BindRc.h"
#include "Msg_InitExternal.h"
#include "Msg_FlightModeReq.h"
#include "Msg_CtrlModeReq.h"
#include "Msg_NewSetpoint.h"



namespace QuadGS
{


Parser::Parser()
{

}

Parser::~Parser()
{
}


QGS_ModuleMsgBase::ptr Parser::parse( std::vector<unsigned char>  data)
{

	QGS_ModuleMsgBase msgBase;

	BinaryIStream is(data);

	QGS_ModuleMsgBase::ptr returnPtr;

	is >> msgBase;
	msgBase.setSkipStreamHeader();
	switch (msgBase.getType())
	{
	case messageTypes_t::Msg_Stop_e:
	{
		Msg_Stop::ptr ptr = std::make_unique<Msg_Stop>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_RegisterName_e:
	{
		Msg_RegisterName::ptr ptr = std::make_unique<Msg_RegisterName>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_Display_e:
	{
		Msg_Display::ptr ptr = std::make_unique<Msg_Display>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_GsAppLog_e:
	{
		Msg_GsAppLog::ptr ptr = std::make_unique<Msg_GsAppLog>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_GsHasLog_e:
	{
		Msg_GsHasLog::ptr ptr = std::make_unique<Msg_GsHasLog>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_GsLogLevel_e:
	{
		Msg_GsLogLevel::ptr ptr = std::make_unique<Msg_GsLogLevel>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_Test_e:
	{
		Msg_Test::ptr ptr = std::make_unique<Msg_Test>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_FindParam_e:
	{
		Msg_FindParam::ptr ptr = std::make_unique<Msg_FindParam>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_GetUiCommands_e:
	{
		Msg_GetUiCommands::ptr ptr = std::make_unique<Msg_GetUiCommands>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_FireUiCommand_e:
	{
		Msg_FireUiCommand::ptr ptr = std::make_unique<Msg_FireUiCommand>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_RegUiCommand_e:
	{
		Msg_RegUiCommand::ptr ptr = std::make_unique<Msg_RegUiCommand>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_UiCommandResult_e:
	{
		Msg_UiCommandResult::ptr ptr = std::make_unique<Msg_UiCommandResult>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_Param_e:
	{
		Msg_Param::ptr ptr = std::make_unique<Msg_Param>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_Log_e:
	{
		Msg_Log::ptr ptr = std::make_unique<Msg_Log>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_AppLog_e:
	{
		Msg_AppLog::ptr ptr = std::make_unique<Msg_AppLog>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_Debug_e:
	{
		Msg_Debug::ptr ptr = std::make_unique<Msg_Debug>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_TestMem_e:
	{
		Msg_TestMem::ptr ptr = std::make_unique<Msg_TestMem>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_TestMemReg_e:
	{
		Msg_TestMemReg::ptr ptr = std::make_unique<Msg_TestMemReg>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_Transmission_e:
	{
		Msg_Transmission::ptr ptr = std::make_unique<Msg_Transmission>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_TestTransmission_e:
	{
		Msg_TestTransmission::ptr ptr = std::make_unique<Msg_TestTransmission>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_FlightMode_e:
	{
		Msg_FlightMode::ptr ptr = std::make_unique<Msg_FlightMode>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_CtrlMode_e:
	{
		Msg_CtrlMode::ptr ptr = std::make_unique<Msg_CtrlMode>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_FcFault_e:
	{
		Msg_FcFault::ptr ptr = std::make_unique<Msg_FcFault>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_Error_e:
	{
		Msg_Error::ptr ptr = std::make_unique<Msg_Error>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_BindRc_e:
	{
		Msg_BindRc::ptr ptr = std::make_unique<Msg_BindRc>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_InitExternal_e:
	{
		Msg_InitExternal::ptr ptr = std::make_unique<Msg_InitExternal>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_FlightModeReq_e:
	{
		Msg_FlightModeReq::ptr ptr = std::make_unique<Msg_FlightModeReq>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_CtrlModeReq_e:
	{
		Msg_CtrlModeReq::ptr ptr = std::make_unique<Msg_CtrlModeReq>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_NewSetpoint_e:
	{
		Msg_NewSetpoint::ptr ptr = std::make_unique<Msg_NewSetpoint>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	
	default:
		break;
	}

	return returnPtr;
}

} /* namespace QuadGS */
