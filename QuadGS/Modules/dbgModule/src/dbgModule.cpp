/*
 * dbgModule.cpp
 *
 * Copyright (C) 2018 Martin Lundh
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

#include "dbgModule.h"
#include "msg_enums.h"
#include "msgAddr.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"
#include "Msg_Display.h"
#include "Msg_BindRc.h"
#include "Msg_FlightModeReq.h"
#include <sstream>

namespace QuadGS {

dbgModule::dbgModule(msgAddr_t name, msgAddr_t dbgAddr):QGS_MessageHandlerBase(name),mDbgAddr(dbgAddr)
{
	mCommands.push_back(UiCommand("dbgModuleSend","Send a message from the dbg module to any other module.",std::bind(&dbgModule::sendUiMsg, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("dbgGetRuntimeStats","Get runtime stats from the FC.",std::bind(&dbgModule::getRuntimeStats, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("rcBind","Bind the spectrum receiver.",std::bind(&dbgModule::BindRc, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("stateChange","Request a state change from the FC.",std::bind(&dbgModule::StateChangeReq, this, std::placeholders::_1)));
}

dbgModule::~dbgModule()
{


}

std::string dbgModule::sendUiMsg(std::string msg)
{
	size_t posEndType = msg.find(" ");
	if(posEndType == std::string::npos)
	{
		return "Provide both message type and address.";
	}
	std::string command = msg.substr(0, posEndType);

	messageTypes_t type = messageTypes_t::Msg_NoType_e;
	for(int i = Msg_NoType_e; i < Msg_LastType_e; i++)
	{
		if(command.compare(messageTypesStr[i]) == 0)
		{
			type = static_cast<messageTypes_t>(i);
			break;
		}
	}

	if(type == messageTypes_t::Msg_NoType_e || type == messageTypes_t::Msg_LastType_e)
	{
		std::stringstream ss;
		ss << "No such type. " << std::endl << "Available types are: " << std::endl;
		for(int i = Msg_NoType_e; i < Msg_LastType_e; i++)
		{
			ss << "	" << messageTypesStr[i] << std::endl;
		}
		return ss.str();
	}

	size_t posEndAddress = msg.find(" ", posEndType+1);
	std::string address = msg.substr(posEndType+1, posEndAddress);



	msgAddr_t addr = msgAddr_t::Unassigned_e;
	for(const auto i : msgAddrStr)
	{
		if(address.compare(i.second) == 0)
		{
			addr = static_cast<msgAddr_t>(i.first);
			break;
		}
	}

	if(addr == msgAddr_t::Unassigned_e)
	{
		std::stringstream ss;
		ss << "No such address. " << std::endl << "Available addresses are: " << std::endl;
		for(unsigned int i = Unassigned_e; i < msgAddrStr.size(); i++)
		{
			ss << "	" << msgAddrStr.at(i) << std::endl;
		}
		return ss.str();
	}

	std::string strResponce;
	switch (type)
	{
	case Msg_NoType_e:
	{
		strResponce = "dbgModule: Warning: Message with no type requested.";
		break;
	}
	case Msg_Test_e:
	{
		Msg_Test::ptr ptr = std::make_unique<Msg_Test>(addr);
		sendMsg(std::move(ptr));
		strResponce = "Message sent.";
		break;
	}
	default:
	{
		strResponce = "dbgModule: Warning: Message with unsupported type requested.";
		break;
	}
	}
	return strResponce;
}

std::string dbgModule::getRuntimeStats(std::string)
{
	Msg_Debug::ptr ptr = std::make_unique<Msg_Debug>(mDbgAddr, DbgCtrl_t::QSP_DebugGetRuntimeStats, "");
	sendMsg(std::move(ptr));

	return "";
}


std::string dbgModule::BindRc(std::string param)
{
	uint8_t quitBind = 0;
	if(!param.empty())
	{
		try
		{
			quitBind = std::stoi(param);
		}
		catch(const std::invalid_argument& e)
		{
			return "Argument not valid.";
		}
		
		if(quitBind != 0 && quitBind != 1)
		{
			return "Argument not valid.";
		}
	}
	Msg_BindRc::ptr ptr = std::make_unique<Msg_BindRc>(FC_Broadcast_e, quitBind);
	sendMsg(std::move(ptr));

	return "";
}

std::string dbgModule::printStateStrings()
{
			std::stringstream ss;
			ss << "No such state. " << std::endl << "Available states are: " << std::endl;
			for (const auto& kv : mapStringToFlightMode)
			{
				ss << "	" << kv.first << std::endl;
			}
			return ss.str();
}


std::string dbgModule::StateChangeReq(std::string param)
{
	if(!param.empty())
	{
		FlightMode_t flightMode;
		try
		{
			flightMode = mapStringToFlightMode.at(param);

		}
		catch(const std::out_of_range & e)
		{
			return printStateStrings();
		}
		
		switch (flightMode)
		{
		    case FlightMode_t::fmode_init:
		        break;

		    case FlightMode_t::fmode_config:
		        break;

		    case FlightMode_t::fmode_arming:
		        break;

		    case FlightMode_t::fmode_disarming:
		        break;

		    default:
				return printStateStrings();

		}
		Msg_FlightModeReq::ptr ptr = std::make_unique<Msg_FlightModeReq>(FC_Broadcast_e, flightMode);
		sendMsg(std::move(ptr));
	}
	else
	{
		return printStateStrings();
	}
	
	return "Request sent";
}


void dbgModule::process(Msg_GetUiCommands* message)
{
	for(UiCommand command : mCommands)
	{
		Msg_RegUiCommand::ptr ptr = std::make_unique<Msg_RegUiCommand>(message->getSource(), command.command, command.doc);
		sendMsg(std::move(ptr));
	}
}



void dbgModule::process(Msg_FireUiCommand* message)
{
	for(UiCommand command : mCommands)
	{
		if(command.command.compare(message->getCommand()) == 0)
		{
			std::string result = command.function(message->getArgs());
			Msg_UiCommandResult::ptr ptr = std::make_unique<Msg_UiCommandResult>(message->getSource(), result);
			sendMsg(std::move(ptr));
			return;
		}
	}
	std::cout << "no such command.";

	Msg_UiCommandResult::ptr ptr = std::make_unique<Msg_UiCommandResult>(message->getSource(), "No such command.");
	sendMsg(std::move(ptr));
	return;
}
void dbgModule::process(Msg_Debug* message)
{
	Msg_Display::ptr ptr = std::make_unique<Msg_Display>(msgAddr_t::GS_GUI_e, message->getPayload());

	sendMsg(std::move(ptr));
}

void dbgModule::process(Msg_FlightMode* message)
{
	std::string flightMode;
	try
	{
		flightMode = mapFlightModeToString.at(static_cast<FlightMode_t>(message->getMode()));
	}
	catch(const std::out_of_range & e)
	{
		mLogger.QuadLog(QuadGS::error, "Received unknown flight mode.");
		return;
	}
	Msg_Display::ptr ptr = std::make_unique<Msg_Display>(msgAddr_t::GS_GUI_e, flightMode);
	sendMsg(std::move(ptr));
}

} /* namespace QuadGS */
