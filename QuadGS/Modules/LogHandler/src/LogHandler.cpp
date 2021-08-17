/*
 * LogHandler.cpp
 *
 *  Created on: Nov 18, 2017
 *      Author: martin
 */

#include "LogHandler.h"

#include <functional>
#include <memory>
#include <iomanip>

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
	mLogFile.open ("LogFile.txt", std::ios_base::app);
	mMapFile.open ("MapFile.txt", std::ios_base::app);

	mLogFile << std::fixed << std::setprecision(9);
	mCommands.push_back(UiCommand("logGetNames","Get name id mapping of logs.",std::bind(&LogHandler::getLogNames, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("logPrintNameMapping","Print name id mapping of logs.",std::bind(&LogHandler::FormatLogMapping, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("logGetEntries","Get runtime logs from QuadFC.",std::bind(&LogHandler::getLogEntries, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("logStopAll","Stop all logging.",std::bind(&LogHandler::stopAllLogs, this, std::placeholders::_1)));
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
		ss << std::left << std::setw(4) <<  it->first << std::setw(15) << it->second.first << std::setw(15) << it->second.second << std::endl;
	}
	return ss.str();
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
				if(!id_str.empty() && !valueType.empty() && !name.empty())
				{
					int id = std::stoi(id_str);
					int type = std::stoi(valueType);
					auto search = mNames.find(id);
					if(search == mNames.end())
					{
						mNames.insert(std::make_pair(id, std::make_pair(name, type)));
						mMapFile << name << "," << id << "," << type << "," << std::endl;

					}
					else
					{
						// already have this, lets make sure it is the same.
						if(mNames[id].first != name)
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
		BinaryIStream is((uint8_t*)payloadStr.c_str(), payloadStr.size());
		while(!is.eof())
		{
			uint32_t id;
			is >> id;
			mLogFile << id;
			mLogFile << ",";

			uint32_t time;
			is >> time;
			mLogFile << time;
			mLogFile << ",";

			if( mNames[id].second == QGS_TreeValue::NodeType_t::fp_16_16_variable_type)
			{
				int32_t data;
				is >> data;
				double dVal = FIXED_TO_DOUBLE(data, MAX16f);

				mLogFile << dVal;
				mLogFile << std::endl;	
			}
			else
			{
				int32_t data;
				is >> data;
				mLogFile << data;
				mLogFile << std::endl;
			}
		}
	}
	break;
	default:
		break;
	}


}

} /* namespace QuadGS */
