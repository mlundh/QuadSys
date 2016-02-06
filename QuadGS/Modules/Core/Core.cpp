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

#include "Core.h"
#include "UiBase.h"
#include "IoBase.h"
#include "QspPayloadRaw.h"
#include "QuadSerialPacket.h"
#include "QuadParamPacket.h"

namespace QuadGS {

Core::Core():
              logger("Core")
{
    mParameters = Parameters::create();
}

Core::~Core()
{

}

Core::ptr Core::create()
{
    ptr tmp = ptr(new Core);
    return tmp;
}

void Core::bind(std::shared_ptr<IoBase> IoPtr)
{
    mIo = IoPtr;
    IoPtr->setReadCallback(std::bind(&QuadGS::Core::msgHandler,shared_from_this(), std::placeholders::_1));

}
void Core::bind(std::shared_ptr<UiBase> UiPtr)
{
    mUi = UiPtr;
    mParameters->RegisterWriteFcn(std::bind(&QuadGS::Core::write, shared_from_this(), std::placeholders::_1));
    UiPtr->registerCommands(getCommands());
    UiPtr->SetCore(getThis());
}


std::string Core::getRuntimeStats(std::string )
{
    QuadSerialPacket::Ptr tmpPacket = QuadSerialPacket::Create(NULL, 0 );
    tmpPacket->SetAddress(QuadSerialPacket::addresses::Debug);
    tmpPacket->SetControl(QuadSerialPacket::DebugControl::GetRuntimeStats);
    write( tmpPacket );
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
    mCommands.push_back(std::make_shared<Command> ("getRuntimeStats",
            std::bind(&Core::getRuntimeStats, shared_from_this(), std::placeholders::_1),
            "Get runtime stats.", Command::ActOn::IO));
    return mCommands;
}

Core::ptr Core::getThis()
{
    return shared_from_this();
}

void Core::write( QuadSerialPacket::Ptr packet)
{
    mIo->write( packet->GetRawData() );
}

void Core::StatusHandler(QuadSerialPacket::Ptr packetPtr)
{
    uint8_t control = packetPtr->GetControl();
    switch (control){
    case QuadSerialPacket::StatusControl::Ack:
        break;
    case QuadSerialPacket::StatusControl::Cont:
    case QuadSerialPacket::StatusControl::BufferOverrun:
    case QuadSerialPacket::StatusControl::Error:
    case QuadSerialPacket::StatusControl::Nack:
    case QuadSerialPacket::StatusControl::NotAllowed:
    case QuadSerialPacket::StatusControl::NotImplemented:
    case QuadSerialPacket::StatusControl::NotValidSlipPacket:
    case QuadSerialPacket::StatusControl::UnexpectedSequence:
        logger.QuadLog(QuadGS::error, "Not valid status response: " + std::to_string(packetPtr->GetControl()));
        break;

    default:
        break;
    }
}
void Core::DebugHandler(QuadSerialPacket::Ptr packetPtr)
{
    uint8_t control = packetPtr->GetControl();
    switch (control){
    case QuadSerialPacket::DebugControl::GetRuntimeStats:
    case QuadSerialPacket::DebugControl::SetRuntimeStats:
        mUi->Display(FormatRuntimeStats(packetPtr->GetPayload()->toString()));
        break;
    case QuadSerialPacket::DebugControl::GetErrorMessages:
        logger.QuadLog(QuadGS::error, "DebugErrorMessage: " + std::to_string(packetPtr->GetControl()));

        break;
    default:
        break;
    }
}

void Core::msgHandler(QuadGS::QuadSerialPacket::Ptr ptr)
{
    uint8_t address = ptr->GetAddress();
    switch (address){
    case QuadSerialPacket::Parameters:
        mParameters->ParameterHandler(ptr);
        break;
    case QuadSerialPacket::FunctionCall:
        // FunctionCallHandler();
        break;
    case QuadSerialPacket::Log:
        // LogHandler();
        break;
    case QuadSerialPacket::Status:
        StatusHandler(ptr);
        break;
    case QuadSerialPacket::Debug:
        DebugHandler(ptr);
        break;
    default:
        logger.QuadLog(severity_level::warning, "Unhandled message: " + std::to_string(address) + " with data: \n" + ptr->GetRawData()->toString());
        break;
    }
}




}
