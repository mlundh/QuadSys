/*
 * LogHandler.cpp
 *
 *  Created on: Nov 18, 2017
 *      Author: martin
 */

#include "../../../QGS_Core/LogHandler/LogHandler.h"

#include <functional>
#include <memory>

#include "../../../QGS_Core/CommandTree/QuadGSTree.h"
#include "../../../QGS_Core/Core.h"
#include "../../../QGS_Core/DataMsg/QCMsgHeader.h"
namespace QuadGS {

LogHandler::LogHandler() :
                                                mNames(),
                                                mWriteFcn()

{
    mLogFile.open ("LogFile.txt", std::ofstream::out);
    mMapFile.open ("MapFile.txt", std::ofstream::out);
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

LogHandler::ptr LogHandler::create()
{
    ptr tmp = ptr(new LogHandler);
    return tmp;
}

void LogHandler::RegisterWriteFcn(WriteFcn fcn)
{
    mWriteFcn = fcn;
}

std::vector< Command::ptr > LogHandler::getCommands()
{
    std::vector<std::shared_ptr < Command > > mCommands;
    mCommands.push_back(std::make_shared<Command> ("logGetNames",
            std::bind(&LogHandler::getLogNames, this, std::placeholders::_1),
            "Get name id mapping of logs.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("logPrintNameMapping",
            std::bind(&LogHandler::FormatLogMapping, this, std::placeholders::_1),
            "Print name id mapping of logs.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("logGetEntries",
            std::bind(&LogHandler::getLogEntries, this, std::placeholders::_1),
            "Get runtime logs from QuadFC.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("logStopAll",
            std::bind(&LogHandler::stopAllLogs, this, std::placeholders::_1),
            "Stop all logging.", Command::ActOn::Core));
    return mCommands;
}

std::string LogHandler::getLogNames(std::string )
{
    QCMsgHeader::ptr header = QCMsgHeader::Create(QCMsgHeader::addresses::Log, QCMsgHeader::LogControl::Name, 0, 0);
    std::shared_ptr<QuadGSMsg> payload;
    if(mWriteFcn)
    {
        mWriteFcn( header, payload );
    }
    else
    {
        return "Can not perform request, no write function registered.";
    }
    return "";
}

std::string LogHandler::getLogEntries(std::string )
{
    QCMsgHeader::ptr header = QCMsgHeader::Create(QCMsgHeader::addresses::Log, QCMsgHeader::LogControl::Entry, 0, 0);
    std::shared_ptr<QuadGSMsg> payload;
    if(mWriteFcn)
    {
        mWriteFcn( header, payload );
    }
    else
    {
        return "Can not perform request, no write function registered.";
    }
    return "";
}

std::string LogHandler::stopAllLogs(std::string )
{
    QCMsgHeader::ptr header = QCMsgHeader::Create(QCMsgHeader::addresses::Log, QCMsgHeader::LogControl::StopAll, 0, 0);
    std::shared_ptr<QuadGSMsg> payload;
    if(mWriteFcn)
    {
        mWriteFcn( header, payload );
    }
    else
    {
        return "Can not perform request, no write function registered.";
    }
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


void LogHandler::Handler(std::shared_ptr<QCMsgHeader> header, std::shared_ptr<QuadLogMsg> payload)
{
    uint8_t control = header->GetControl();
    switch (control)
    {
    case QCMsgHeader::LogControl::Name:
    {
        std::string payloadStr = payload->GetPayload();
        while(!payloadStr.empty())
        {
            std::string module = QuadGSTree::RemoveModuleString(payloadStr);

            if(!module.empty())
            {
                std::string name = QuadGSTree::GetModuleName(module);
                std::string valueType = QuadGSTree::GetValueTypeString(module);
                std::string id_str = QuadGSTree::GetValueString(module);
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
    case QCMsgHeader::LogControl::Entry:
    {
        std::string payloadStr = payload->GetPayload();
        bool end = false;
        if(payloadStr.empty())
        {
            end = true;
        }
        while(!payloadStr.empty())
        {
            std::string module = QuadGSTree::RemoveModuleString(payloadStr);
            for(int i = 0; i < NR_LOG_FIELDS; i++)
            {
                std::string value = QuadGSTree::RemoveValueString(module);
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
