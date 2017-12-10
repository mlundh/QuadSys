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

#include "QGS_Msg.h"
#include "LogHandler.h"
#include "Parameters.h"
#include "QGS_MsgHeader.h"

#include "QGS_CoreInterface.h"
#include "QGS_UiInterface.h"
#include "QGS_IoInterface.h"

#include "QGS_ParamMsg.h"
#include "QGS_DebugMsg.h"

namespace QuadGS {

QGS_CoreInterface::QGS_CoreInterface()
:logger("Core")
,mIo()
,mUi()
{
    mParameters = Parameters::create();
    mLogHandler = LogHandler::create();
}

QGS_CoreInterface::~QGS_CoreInterface()
{

}

QGS_CoreInterface* QGS_CoreInterface::create()
{
	QGS_CoreInterface* tmp = new QGS_CoreInterface;
    return tmp;
}

void QGS_CoreInterface::bind(QGS_IoInterface* IoPtr)
{
    mIo = IoPtr;
    IoPtr->setMessageCallback(std::bind(&QuadGS::QGS_CoreInterface::msgHandler, this, std::placeholders::_1, std::placeholders::_2));

}
void QGS_CoreInterface::bind(QGS_UiInterface* UiPtr)
{
    mUi = UiPtr;
    mParameters->RegisterWriteFcn(std::bind(&QuadGS::QGS_CoreInterface::write, this, std::placeholders::_1, std::placeholders::_2));
    mLogHandler->RegisterWriteFcn(std::bind(&QuadGS::QGS_CoreInterface::write, this, std::placeholders::_1, std::placeholders::_2));
    UiPtr->registerCommands(getCommands());
    UiPtr->bind(this);
}

std::string QGS_CoreInterface::getRuntimeStats(std::string )
{
	QGS_MsgHeader::ptr tmpPacket = QGS_MsgHeader::Create(QGS_MsgHeader::addresses::Debug, QGS_MsgHeader::DebugControl::GetRuntimeStats, 0, 0);
    std::shared_ptr<QGS_Msg> Payload;
    write( tmpPacket, Payload);
    return "";
}


std::string QGS_CoreInterface::FormatRuntimeStats(std::string runtimeStats)
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


std::vector< QGS_UiCommand::ptr > QGS_CoreInterface::getCommands()
{
    std::vector<std::shared_ptr < QGS_UiCommand > > mCommands;
    mCommands = mParameters->getCommands();
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("runtimeStatsGet",
            std::bind(&QGS_CoreInterface::getRuntimeStats, this, std::placeholders::_1),
            "Get runtime stats.", QGS_UiCommand::ActOn::IO));
    std::vector<std::shared_ptr < QGS_UiCommand > > mCommandsLogHandler;
    mCommandsLogHandler = mLogHandler->getCommands();
    mCommands.insert( mCommands.end(), mCommandsLogHandler.begin(), mCommandsLogHandler.end() );
    return mCommands;
}

void QGS_CoreInterface::write( QGS_MsgHeader::ptr header, QGS_Msg::QuadGSMsgPtr payload)
{
    mIo->write( header, payload );
}

void QGS_CoreInterface::StatusHandler(QGS_MsgHeader::ptr packetPtr)
{
    uint8_t control = packetPtr->GetControl();
    switch (control){
    case QGS_MsgHeader::StatusControl::Ack:
        break;
    case QGS_MsgHeader::StatusControl::Cont:
    case QGS_MsgHeader::StatusControl::BufferOverrun:
    case QGS_MsgHeader::StatusControl::Error:
    case QGS_MsgHeader::StatusControl::Nack:
    case QGS_MsgHeader::StatusControl::NotAllowed:
    case QGS_MsgHeader::StatusControl::NotImplemented:
    case QGS_MsgHeader::StatusControl::NotValidSlipPacket:
    case QGS_MsgHeader::StatusControl::UnexpectedSequence:
        logger.QuadLog(QuadGS::error, "Not valid status response: " + std::to_string(packetPtr->GetControl()));
        break;

    default:
        break;
    }
}
void QGS_CoreInterface::DebugHandler(QGS_MsgHeader::ptr header, QGS_DebugMsg::ptr payload)
{
    uint8_t control = header->GetControl();
    switch (control){
    case QGS_MsgHeader::DebugControl::GetRuntimeStats:
    case QGS_MsgHeader::DebugControl::SetRuntimeStats:
        mUi->Display(FormatRuntimeStats(payload->GetPayload()));
        break;
    case QGS_MsgHeader::DebugControl::GetErrorMessages:
        logger.QuadLog(QuadGS::error, "DebugErrorMessage: " + std::to_string(header->GetControl()));

        break;
    default:
        break;
    }
}


void QGS_CoreInterface::msgHandler(std::shared_ptr<QGS_MsgHeader> header, std::shared_ptr<QGS_Msg> payload)
{
    uint8_t address = header->GetAddress();
    switch (address){
    case QGS_MsgHeader::addresses::Parameters:
        mParameters->ParameterHandler(header, std::dynamic_pointer_cast<QGSParamMsg>(payload));
        break;
    case QGS_MsgHeader::addresses::FunctionCall:
        // FunctionCallHandler();
        break;
    case QGS_MsgHeader::addresses::Log:
        mLogHandler->Handler(header, std::dynamic_pointer_cast<QGS_LogMsg>(payload));
        break;
    case QGS_MsgHeader::addresses::Status:
        StatusHandler(header);
        break;
    case QGS_MsgHeader::addresses::Debug:
        DebugHandler(header, std::dynamic_pointer_cast<QGS_DebugMsg>(payload));
        break;
    default:
        logger.QuadLog(severity_level::warning, "Unhandled message: " + std::to_string(address) + " with data: \n" + payload->toString());
        break;
    }
}




}
