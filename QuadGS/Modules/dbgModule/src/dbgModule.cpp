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
#include "msgAddr.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"
#include "Msg_Display.h"
#include <sstream>

namespace QuadGS {

dbgModule::dbgModule(msgAddr_t name):QGS_MessageHandlerBase(name)
{
	mCommands.push_back(UiCommand("dbgModuleSend","Send a message from the dbg module to any other module.",std::bind(&dbgModule::sendUiMsg, this, std::placeholders::_1)));
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



	msgAddr_t addr = msgAddr_t::Unassigned;
	for(int i = msgAddr_t::Unassigned ; i < msgAddr_t::Last; i++)
	{
		if(address.compare(msgAddrStr[i]) == 0)
		{
			addr = static_cast<msgAddr_t>(i);
			break;
		}
	}

	if(addr == msgAddr_t::Unassigned || addr == msgAddr_t::Last)
	{
		std::stringstream ss;
		ss << "No such address. " << std::endl << "Available addresses are: " << std::endl;
		for(int i = Unassigned; i < Last; i++)
		{
			ss << "	" << msgAddrStr[i] << std::endl;
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

void dbgModule::process(Msg_Test* message)
{
	Msg_Test::ptr ptr = std::make_unique<Msg_Test>(msgAddr_t::GUI_e);
	sendMsg(std::move(ptr));
}
} /* namespace QuadGS */
