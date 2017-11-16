/*
 * Core.cpp
 *
 * Copyright (C) 2015 Martin Lundh
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
#include <memory>
#include "Core.h"

#include "QCMsgHeader.h"
#include "QuadParamPacket.h"
#include "QuadDebugMsg.h"
#include "QuadGSMsg.h"
#include "UiBase.h"
#include "IoBase.h"
#include "LogHandler.h"
#include "Parameters.h"

namespace QuadGS {

Core::Core()
:logger("Core")
,mIo()
,mUi()
{
    mParameters = Parameters::create();
    mLogHandler = LogHandler::create();
}

Core::~Core()
{

}

Core* Core::create()
{
	Core* tmp = new Core;
    return tmp;
}

void Core::bind(IoBase* IoPtr)
{
    mIo = IoPtr;
    IoPtr->setMessageCallback(std::bind(&QuadGS::Core::msgHandler, this, std::placeholders::_1, std::placeholders::_2));

}
void Core::bind(UiBase* UiPtr)
{
    mUi = UiPtr;
    mParameters->RegisterWriteFcn(std::bind(&QuadGS::Core::write, this, std::placeholders::_1, std::placeholders::_2));
    mLogHandler->RegisterWriteFcn(std::bind(&QuadGS::Core::write, this, std::placeholders::_1, std::placeholders::_2));
    UiPtr->registerCommands(getCommands());
    UiPtr->SetCore(this);
}

std::string Core::getRuntimeStats(std::string )
{
	QCMsgHeader::ptr tmpPacket = QCMsgHeader::Create(QCMsgHeader::addresses::Debug, QCMsgHeader::DebugControl::GetRuntimeStats, 0, 0);
    std::shared_ptr<QuadGSMsg> Payload;
    write( tmpPacket, Payload);
    return "";
}


std::string Core::FormatRuntimeStats(std::string runtimeStats)
{
    std::stringstream          lineStream(runtimeStats);
    std::string                cell;
    std::stringstream          ss;
    std::vector<std::string>   parts;

    // split into parts defined by ","
    while(std::getline(lineStream,cell,','))
    {
        parts.push_back(cell);
    }
    ss << std::left << std::setw(11) << "Name";
    ss << std::left << std::setw(12) << "CurrPrio";
    ss << std::left << std::setw(12) << "stackHigh";
    ss << std::left << std::setw(12) << "TotalTime";
    ss << std::left << std::setw(12) << "RelativeTime";
    for(unsigned int i = 0; i < parts.size(); i++)
    {
        ss << std::left << std::setw(12) << parts[i];
    }
    return ss.str();
}


std::vector< Command::ptr > Core::getCommands()
{
    std::vector<std::shared_ptr < Command > > mCommands;
    mCommands = mParameters->getCommands();
    mCommands.push_back(std::make_shared<Command> ("runtimeStatsGet",
            std::bind(&Core::getRuntimeStats, this, std::placeholders::_1),
            "Get runtime stats.", Command::ActOn::IO));
    std::vector<std::shared_ptr < Command > > mCommandsLogHandler;
    mCommandsLogHandler = mLogHandler->getCommands();
    mCommands.insert( mCommands.end(), mCommandsLogHandler.begin(), mCommandsLogHandler.end() );
    return mCommands;
}

void Core::write( QCMsgHeader::ptr header, QuadGSMsg::QuadGSMsgPtr payload)
{
    mIo->write( header, payload );
}

void Core::StatusHandler(QCMsgHeader::ptr packetPtr)
{
    uint8_t control = packetPtr->GetControl();
    switch (control){
    case QCMsgHeader::StatusControl::Ack:
        break;
    case QCMsgHeader::StatusControl::Cont:
    case QCMsgHeader::StatusControl::BufferOverrun:
    case QCMsgHeader::StatusControl::Error:
    case QCMsgHeader::StatusControl::Nack:
    case QCMsgHeader::StatusControl::NotAllowed:
    case QCMsgHeader::StatusControl::NotImplemented:
    case QCMsgHeader::StatusControl::NotValidSlipPacket:
    case QCMsgHeader::StatusControl::UnexpectedSequence:
        logger.QuadLog(QuadGS::error, "Not valid status response: " + std::to_string(packetPtr->GetControl()));
        break;

    default:
        break;
    }
}
void Core::DebugHandler(QCMsgHeader::ptr header, QuadDebugMsg::ptr payload)
{
    uint8_t control = header->GetControl();
    switch (control){
    case QCMsgHeader::DebugControl::GetRuntimeStats:
    case QCMsgHeader::DebugControl::SetRuntimeStats:
        mUi->Display(FormatRuntimeStats(payload->GetPayload()));
        break;
    case QCMsgHeader::DebugControl::GetErrorMessages:
        logger.QuadLog(QuadGS::error, "DebugErrorMessage: " + std::to_string(header->GetControl()));

        break;
    default:
        break;
    }
}


void Core::msgHandler(std::shared_ptr<QCMsgHeader> header, std::shared_ptr<QuadGSMsg> payload)
{
    uint8_t address = header->GetAddress();
    switch (address){
    case QCMsgHeader::addresses::Parameters:
        mParameters->ParameterHandler(header, std::dynamic_pointer_cast<QuadParamPacket>(payload));
        break;
    case QCMsgHeader::addresses::FunctionCall:
        // FunctionCallHandler();
        break;
    case QCMsgHeader::addresses::Log:
        mLogHandler->Handler(header, std::dynamic_pointer_cast<QuadLogMsg>(payload));
        break;
    case QCMsgHeader::addresses::Status:
        StatusHandler(header);
        break;
    case QCMsgHeader::addresses::Debug:
        DebugHandler(header, std::dynamic_pointer_cast<QuadDebugMsg>(payload));
        break;
    default:
        logger.QuadLog(severity_level::warning, "Unhandled message: " + std::to_string(address) + " with data: \n" + payload->toString());
        break;
    }
}




}
