/*
 * LogHandler.cpp
 *
 *  Created on: Nov 18, 2017
 *      Author: martin
 */

#include "LogHandler.h"

#include <functional>
#include <memory>

#include "msg_enums.h"
#include "QGS_Tree.h"
#include "QGS_Module.h"
#include "Msg_Log.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"

namespace QuadGS {

LogHandler::UiCommand::UiCommand(std::string command,  std::string doc, UiFcn function):
				command(command), doc(doc), function(function)
{

}
LogHandler::LogHandler(msgAddr_t name, msgAddr_t loggerAddr)
:QGS_MessageHandlerBase(name)
,mNames()
,mLoggerAddr(loggerAddr)
{
	mLogFile.open ("LogFile.txt", std::ofstream::out);
	mMapFile.open ("MapFile.txt", std::ofstream::out);

	mCommands.push_back(UiCommand("logGetNames","Get name id mapping of logs.",std::bind(&LogHandler::getLogNames, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("logPrintNameMapping","Print name id mapping of logs.",std::bind(&LogHandler::FormatLogMapping, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("logGetEntries","Get runtime logs from QuadFC.",std::bind(&LogHandler::getLogEntries, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("logStopAll","Stop all logging.",std::bind(&LogHandler::stopAllLogs, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("appLogLevel","Set the log level of the app.",std::bind(&LogHandler::SetAppLogLevel, this, std::placeholders::_1)));
}
LogHandler::~LogHandler()
{
	if (mLogFile.is_open())
	{
		mLogFile.close();
	}
	if (mMapFile.is_open())
	{
		mMapFile.close();
	}
}

std::string LogHandler::getLogNames(std::string )
{
	Msg_Log::ptr ptr = std::make_unique<Msg_Log>(mLoggerAddr, LogCtrl_t::Name,"");
	sendMsg(std::move(ptr));
	return "";
}

std::string LogHandler::getLogEntries(std::string )
{
	Msg_Log::ptr ptr = std::make_unique<Msg_Log>(mLoggerAddr, LogCtrl_t::Entry,"");
	sendMsg(std::move(ptr));
	return "";
}

std::string LogHandler::stopAllLogs(std::string )
{
	Msg_Log::ptr ptr = std::make_unique<Msg_Log>(mLoggerAddr, LogCtrl_t::StopAll,"");
	sendMsg(std::move(ptr));
	return "";
}

std::string LogHandler::FormatLogMapping(std::string)
{
	if(mNames.size() == 0)
	{
		return "No mapping registered.";
	}
	std::stringstream          ss;
	std::vector<std::string>   parts;
	for (auto it = mNames.begin(); it != mNames.end(); ++it) {
		ss << std::left << std::setw(12) <<  it->first << std::setw(12) << it->second << std::endl;
	}
	return ss.str();
}

std::string LogHandler::SetAppLogLevel(std::string string)
{
	return AppLog::setLogLevelFromStr(string);
}



void LogHandler::process(Msg_GetUiCommands* message)
{
	for(UiCommand command : mCommands)
	{
		Msg_RegUiCommand::ptr ptr = std::make_unique<Msg_RegUiCommand>(message->getSource(), command.command, command.doc);
		sendMsg(std::move(ptr));
	}
}
void LogHandler::process(Msg_FireUiCommand* message)
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
void LogHandler::process(Msg_Log* message)
{
	switch (message->getControl())
	{
	case LogCtrl_t::Name:
	{
		std::string payloadStr = message->getPayload();
		while(!payloadStr.empty())
		{
			std::string module = QGS_Tree::RemoveModuleString(payloadStr);

			if(!module.empty())
			{
				std::string name = QGS_Tree::GetModuleName(module);
				std::string valueType = QGS_Tree::GetValueTypeString(module);
				std::string id_str = QGS_Tree::GetValueString(module);
				if(!id_str.empty() && !valueType.empty())
				{
					int id = std::stoi(id_str);
					int type = std::stoi(valueType);
					auto search = mNames.find(name);
					if(search == mNames.end())
					{
						mNames[name] = id;
						mMapFile << name << "," << id << "," << type << std::endl;

					}
					else
					{
						// already have this, lets make sure it is the same.
						if(mNames[name] != id)
						{
							// TODO clear the file!
							mNames.clear();
							throw std::runtime_error("Conflicting name-id mapping for log entries. Discarding all names.");
						}
					}
				}
				else
				{
					throw std::runtime_error("No ID given.");
				}

			}
		}
	}
	break;
	case LogCtrl_t::Entry:
	{
		std::string payloadStr = message->getPayload();
		bool end = false;
		if(payloadStr.empty())
		{
			end = true;
		}
		while(!payloadStr.empty())
		{
			std::string module = QGS_Tree::RemoveModuleString(payloadStr);
			for(int i = 0; i < NR_LOG_FIELDS; i++)
			{
				std::string value = QGS_Tree::RemoveValueString(module);
				mLogFile << value;
				if(i < NR_LOG_FIELDS - 1)
				{
					mLogFile << ",";
				}
				else
				{
					mLogFile << std::endl;
				}

			}
		}
		if(!end)
		{
			getLogEntries("");
		}
	}
	break;
	default:
		break;
	}


}

} /* namespace QuadGS */
