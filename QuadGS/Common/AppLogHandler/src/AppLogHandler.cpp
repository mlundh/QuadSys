/*
* AppLogHandler.cpp
*
* Copyright (C) 2021  Martin Lundh
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

#include "AppLogHandler.h"

#include <iostream>
#include <fstream>
#include <algorithm> 
#include <cctype>
#include <locale>

#include "msg_enums.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"
namespace QuadGS {



// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

static QuadGS::msgAddr_t addressFromStr(std::string address)
{
	
	trim(address);
	QuadGS::msgAddr_t addr = static_cast<msgAddr_t>(msgAddrfromStr.at(address));

	return addr;
}

static std::string addressToStr(QuadGS::msgAddr_t addr)
{
	std::string str = msgAddrStr.at(addr);
	return str;
}



AppLogHandler::AppLogHandler(msgAddr_t name)
:QGS_MessageHandlerBase(name)
{
	mAppLogFile.open ("AppLogFile.txt", std::ofstream::out);

	mCommands.push_back(UiCommand("appSetLogLevl","Set log level of module",std::bind(&AppLogHandler::setLogLevelFromStr, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("appGetLogLevl","Get log level of module",std::bind(&AppLogHandler::getLogLevel, this, std::placeholders::_1)));

}
AppLogHandler::~AppLogHandler()
{
	if (mAppLogFile.is_open())
	{
		mAppLogFile.close();
	}
}

void AppLogHandler::process(Msg_GetUiCommands* message)
{
	for(UiCommand command : mCommands)
	{
		Msg_RegUiCommand::ptr ptr = std::make_unique<Msg_RegUiCommand>(message->getSource(), command.command, command.doc);
		sendMsg(std::move(ptr));
	}
	// this function will be called after complete binding of the topology. 
	Msg_GsHasLog::ptr ptr = std::make_unique<Msg_GsHasLog>(msgAddr_t::GS_Broadcast_e);
	sendMsg(std::move(ptr));
}

void AppLogHandler::process(Msg_FireUiCommand* message)
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

void AppLogHandler::process(Msg_GsAppLog* message)
{
	// TODO write to file and screen.
	std::cout << message->getMessage();
	return;
}

void AppLogHandler::process(Msg_GsHasLog* message)
{
	modulesWithLog.insert(message->getSource());
}

void AppLogHandler::process(Msg_GsLogLevel* message)
{
	std::cout << addressToStr(message->getSource()) << " " << AppLog::mSeverityStrings[message->getLoglevel()] << std::endl;
}


QuadGS::log_level AppLogHandler::logLevelFromStr(std::string level)
{
	QuadGS::log_level logLevel = QuadGS::invalid;
	trim(level);

	for(unsigned int i = 0; i < AppLog::mSeverityStrings.size(); i++)
	{
		if(!level.compare(AppLog::mSeverityStrings[i]))
		{
			logLevel = static_cast<log_level>(i);
			break;
		}
	}
	return logLevel;
}



std::string AppLogHandler::setLogLevelFromStr(std::string level)
{
	std::array<std::string, 2> tokens;
	for(int i = 0; i < 2; i++) 
	{
		size_t pos = 0;
		pos = level.find(" ");
		tokens[i] = level.substr(0, pos);
		level.erase(0, pos + 1);

		if(pos == std::string::npos && i != 1)
		{
			std::stringstream ss;
			ss << "Please provide all arguments separated with space: [address] [logLevel], allowed addresses are : " << std::endl
				<< listAddresses() << std::endl  << "Allowed logLevles are: " << std::endl << listLogLevels() << std::endl;
			return ss.str();
		}
	}


	QuadGS::msgAddr_t address;
	
	try
	{
		address = addressFromStr(tokens[0]);
	}
	catch(const std::out_of_range& e)
	{
		return "Invalid address";
	}
	 

	QuadGS::log_level lvl = AppLogHandler::logLevelFromStr(tokens[1]);
	std::string result;
	if(lvl != log_level::invalid)
	{
		Msg_GsLogLevel::ptr ptr = std::make_unique<Msg_GsLogLevel>(static_cast<QuadGS::msgAddr_t>(address), appLogLL::appLog_setLL, static_cast<uint32_t>(lvl));
		sendMsg(std::move(ptr));
		result = "";
	}
	else
	{
		std::stringstream ss;
		ss << "Invalid log level: " << level << " ." << std::endl << " Allowed options are: " << std::endl << listLogLevels() << std::endl;
		result = ss.str();
	}
	return result;
}

std::string AppLogHandler::getLogLevel(std::string address)
{
	trim(address);
	QuadGS::msgAddr_t addr;
	if(address.empty())
	{
		addr = GS_Broadcast_e;
	}
	else
	{
		try
		{
			 addr = addressFromStr(address);
		}
		catch(const std::out_of_range& e)
		{
			return "Not a valid address.";
		}
	}

	Msg_GsLogLevel::ptr ptr = std::make_unique<Msg_GsLogLevel>(addr, appLogLL::appLog_getLL, 0);
	sendMsg(std::move(ptr));
	return "";
}


std::string AppLogHandler::listLogLevels()
{
		std::stringstream ss;
		for(auto i : AppLog::mSeverityStrings)
		{
			ss << i << std::endl;
		}
		return ss.str();
}

std::string AppLogHandler::listAddresses()
{
	std::stringstream ss;
	
	for(auto i : modulesWithLog)
	{
		ss << addressToStr(i) << std::endl;
	}
	return ss.str();
}


}//namespace 