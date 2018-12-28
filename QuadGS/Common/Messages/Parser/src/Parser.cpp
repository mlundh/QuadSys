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
#include "Msg_DebugIo.h"
#include "Msg_ParamIo.h"
#include "Msg_LogIo.h"
#include "Msg_Param.h"
#include "Msg_Log.h"
#include "Msg_FindParam.h"
#include "Msg_GetUiCommands.h"
#include "Msg_FireUiCommand.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"
#include "Msg_IoWrapper.h"
#include "Msg_Transmission.h"



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
	case messageTypes_t::Msg_DebugIo_e:
	{
		Msg_DebugIo::ptr ptr = std::make_unique<Msg_DebugIo>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_ParamIo_e:
	{
		Msg_ParamIo::ptr ptr = std::make_unique<Msg_ParamIo>(msgBase);
		ptr->setSkipStreamHeader();
		is >> *ptr;
		returnPtr = std::move(ptr);
		break;
	}
	case messageTypes_t::Msg_LogIo_e:
	{
		Msg_LogIo::ptr ptr = std::make_unique<Msg_LogIo>(msgBase);
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
	case messageTypes_t::Msg_IoWrapper_e:
	{
		Msg_IoWrapper::ptr ptr = std::make_unique<Msg_IoWrapper>(msgBase);
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
	
	default:
		break;
	}

	return returnPtr;
}

} /* namespace QuadGS */
