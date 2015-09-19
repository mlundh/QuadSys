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

#include <boost/algorithm/string.hpp>
namespace QuadGS {

Core::Core():
                                 logger("Core")
{
    mTree = QuadGSTree::ptr();
    mCurrentBranch = QuadGSTree::ptr();
    mTmpBranch = QuadGSTree::ptr();
    mSavedBranch = QuadGSTree::ptr();

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
    UiPtr->registerCommands(getCommands());
    UiPtr->SetCore(getThis());
}

bool Core::UpdateTmp(std::string& path)
{
    if(!mTree)
    {
        throw std::runtime_error("No tree registered");
    }
    size_t pos = path.find(QuadGSTree::mBranchDelimiter);
    if (pos == 0) // Update from root.
    {

        mTmpBranch = mTree;

        path.erase(0, pos + QuadGSTree::mBranchDelimiter.length());

        if(mTmpBranch->NeedUpdate(path))
        {
            return true;
        }
        //else remove module from string and continue.
        QuadGSTree::RemoveModuleString(path);
    }
    else 
    {
        mTmpBranch = mCurrentBranch;
    }
    while(!path.empty())
    {
        // Find will return shared_ptr to next element, or current if there is work to do in current node.
        QuadGSTree::ptr tmp = mTmpBranch->Find(path);

        // Node not found, return.
        if(!tmp)
        {
            return false;
        }
        else
        {
            // Change to branch.
            mTmpBranch = tmp;

            // If branch need update then return.
            if(mTmpBranch->NeedUpdate(path))
            {
                return true;
            }
            //else remove module from string and continue.
            QuadGSTree::RemoveModuleString(path);
        }
    }
    //we found the whole path! Return true, we found a node.
    return true;
}

void Core::SaveBranch()
{
    mSavedBranch = mCurrentBranch;
}

void Core::RestoreBranch()
{
    mCurrentBranch = mSavedBranch;
}

bool Core::ChangeBranch(std::string& path)
{
    bool found = UpdateTmp(path);
    mCurrentBranch = mTmpBranch;
    return found;
}

std::string Core::ChangeBranchCmd(std::string path)
{
    ChangeBranch(path);
    if(!path.empty())
    {
        throw std::runtime_error("Path does not exist: " + path + ".");
    }
    return "";
}

std::string Core::PrintCurrentPath(std::string)
{
    if(!mTree)
    {
        throw std::runtime_error("No tree registered");
    }
    std::string Path = mCurrentBranch->GetName();
    QuadGSTree* tmp = mCurrentBranch->GetParent();
    while(tmp)
    {
        std::string parentName = tmp->GetName();
        Path = parentName + QuadGSTree::mBranchDelimiter + Path;
        tmp = tmp->GetParent();
    }
    Path = "/" + Path;
    return Path;
}

std::string Core::list(std::string path)
{
    UpdateTmp(path);
    if(!path.empty())
    {
        throw std::runtime_error("Path does not exist: " + path + ".");
    }
    std::string resp = mTmpBranch->ListChildren();
    return resp;
}


std::string Core::get(std::string path)
{
    UpdateTmp(path);
    if(!path.empty())
    {
        throw std::runtime_error("Get command can not contain set information.");
    }
    std::string value = mTmpBranch->GetValue();
    return value;
}
std::string Core::set(std::string path)
{
    std::exception_ptr eptr;
    SaveBranch();
    try
    {
        while(!path.empty())
        {
            ChangeBranch(path);
            if(!path.empty())
            {
                mTmpBranch->SetValue(path);
                QuadGSTree::RemoveModuleString(path);
            }
        }
    }
    catch(...)
    {
        eptr = std::current_exception();
        RestoreBranch();
        std::rethrow_exception(eptr);
    }
    RestoreBranch();
    return "";
}


std::string Core::SetAndRegister(std::string path)
{
    std::exception_ptr eptr;
    SaveBranch();
    try
    {
        while(!path.empty())
        {
            bool found = ChangeBranch(path);
            if(!path.empty())
            {
                if(!found)
                {
                    mTmpBranch->Register(path);
                }
                else
                {
                    mTmpBranch->SetValue(path);
                    QuadGSTree::RemoveModuleString(path);
                }
            }
        }
    }
    catch(...)
    {
        eptr = std::current_exception();
        RestoreBranch();
        std::rethrow_exception(eptr);
    }
    RestoreBranch();
    return "";
}


std::string Core::writeRawCmd(std::string data)
{
    // first there should be an address value, then control then data.
    std::istringstream iss(data);
    std::string address;

    iss >> address;
    data.erase(data.begin(), data.begin() + address.size());
    boost::trim(data);

    std::string control;
    iss >> control;
    data.erase(data.begin(), data.begin() + control.size());
    boost::trim(data);

    if(address.empty() || control.empty() || data.empty())
    {
        return "format: [address] [control] [data]. Example:\"1 1 /root/tmp<5>[8]/test[3]\"";
    }

    uint8_t iAddress;
    uint8_t iControl;
    try
    {
        iAddress = static_cast<uint8_t>(std::stoi(address));
        iControl = static_cast<uint8_t>(std::stoi(control));
    }
    catch (const std::invalid_argument& e)
    {
        return "Invalid address or control.";
    }
    catch (const std::out_of_range& e)
    {
        return "Address or control to large.";
    }

    QuadSerialPacket::Ptr tmp = QuadSerialPacket::Create(reinterpret_cast<const uint8_t*>(data.c_str()),static_cast<uint16_t>(data.length()) );
    tmp->SetAddress(iAddress);
    tmp->SetControl(iControl);
    write( tmp );
    return "";
}

std::string Core::writeCmd(std::string path_dump)
{
    bool cont = true;
    std::vector<size_t> StartPosition(QuadGSTree::mMaxDepth,0);
    uint8_t SequenceNumber = 0;
    while(cont)
    {
        UpdateTmp(path_dump);
        std::string Path;
        QuadGSTree* tmp = mTmpBranch->GetParent();
        while(tmp)
        {
            std::string parentString = tmp->GetNodeString();
            Path = QuadGSTree::mBranchDelimiter + parentString + Path;
            tmp = tmp->GetParent();
        }
        unsigned int Depth = 0;
        cont = !(mTmpBranch->DumpTree(Path, StartPosition, Depth, 256));
        Path += "/";
        QuadParamPacket::Ptr tmpPacket = QuadParamPacket::Create(reinterpret_cast<const uint8_t*>(Path.c_str()),static_cast<uint16_t>(Path.length()) );
        tmpPacket->SetSequenceNumber(SequenceNumber++);
        tmpPacket->SetAddress(QuadSerialPacket::addresses::Parameters);
        tmpPacket->SetControl(QuadSerialPacket::ParametersControl::SetTree);

        write( std::static_pointer_cast<QuadSerialPacket>(tmpPacket) );
    }
    return "";
}

std::string Core::requestUpdateCmd(std::string )
{
    RequestTree();
    return "";
}

std::string Core::saveParamCmd(std::string )
{
    QuadParamPacket::Ptr tmpPacket = QuadParamPacket::Create(NULL, 0 );
    tmpPacket->SetSequenceNumber(0);
    tmpPacket->SetAddress(QuadParamPacket::addresses::Parameters);
    tmpPacket->SetControl(QuadParamPacket::ParametersControl::Save);
    write( std::static_pointer_cast<QuadSerialPacket>(tmpPacket) );
    return "";
}

std::string Core::loadParamCmd(std::string )
{
    QuadParamPacket::Ptr tmpPacket = QuadParamPacket::Create(NULL, 0 );
    tmpPacket->SetSequenceNumber(0);
    tmpPacket->SetAddress(QuadParamPacket::addresses::Parameters);
    tmpPacket->SetControl(QuadParamPacket::ParametersControl::Load);
    write( std::static_pointer_cast<QuadSerialPacket>(tmpPacket) );
    return "";
}

std::string Core::dump(std::string path)
{
    UpdateTmp(path);
    return  mTmpBranch->DumpTreeFormatted(0);
}
std::vector< Command::ptr > Core::getCommands()
{
    std::vector<std::shared_ptr < Command > > mCommands;
    mCommands.push_back(std::make_shared<Command> ("cd",
            std::bind(&Core::ChangeBranchCmd, shared_from_this(), std::placeholders::_1),
            "Change branch", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("pwd",
            std::bind(&Core::PrintCurrentPath, shared_from_this(), std::placeholders::_1),
            "Print current branch", Command::ActOn::Core));     
    mCommands.push_back(std::make_shared<Command> ("ls",
            std::bind(&Core::list, shared_from_this(), std::placeholders::_1),
            "List children on branch", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("set",
            std::bind(&Core::set, shared_from_this(), std::placeholders::_1),
            "Set value of the command tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("get",
            std::bind(&Core::get, shared_from_this(), std::placeholders::_1),
            "Get value of the command tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("RegisterTree",
            std::bind(&Core::SetAndRegister, shared_from_this(), std::placeholders::_1),
            "SetAndRegister a new value in the tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("dumpTree",
            std::bind(&Core::dump, shared_from_this(), std::placeholders::_1),
            "Dump the command tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("write",
            std::bind(&Core::writeCmd, shared_from_this(), std::placeholders::_1),
            "Write tree to port, relative to current node and path.", Command::ActOn::IO));
    mCommands.push_back(std::make_shared<Command> ("writeRawData",
            std::bind(&Core::writeRawCmd, shared_from_this(), std::placeholders::_1),
            "Write raw data to the serial port.", Command::ActOn::IO));
    mCommands.push_back(std::make_shared<Command> ("read",
            std::bind(&Core::requestUpdateCmd, shared_from_this(), std::placeholders::_1),
            "Request an update from FC.", Command::ActOn::IO));
    mCommands.push_back(std::make_shared<Command> ("save",
            std::bind(&Core::saveParamCmd, shared_from_this(), std::placeholders::_1),
            "Tell fc to save current parameters.", Command::ActOn::IO));
    mCommands.push_back(std::make_shared<Command> ("load",
            std::bind(&Core::loadParamCmd, shared_from_this(), std::placeholders::_1),
            "Tell fc to load saved parameters.", Command::ActOn::IO));
    return mCommands;
}

Core::ptr Core::getThis()
{
    return shared_from_this();
}

void Core::FindPartial(std::string& name, std::vector<std::string>& vec)
{
    UpdateTmp(name);
    mTmpBranch->FindPartial(name, vec);
}

void Core::write( QuadSerialPacket::Ptr packet)
{
    mIo->write( packet->GetRawData() );
}

void Core::RequestTree()
{
    QuadParamPacket::Ptr tmpPacket = QuadParamPacket::Create(NULL, 0 );
    tmpPacket->SetSequenceNumber(0);
    tmpPacket->SetAddress(QuadSerialPacket::addresses::Parameters);
    tmpPacket->SetControl(QuadSerialPacket::ParametersControl::GetTree);
    write( std::static_pointer_cast<QuadSerialPacket>(tmpPacket) );
}

void Core::ParameterHandler(QuadSerialPacket::Ptr packetPtr)
{
    QuadParamPacket::Ptr paramPacket = QuadParamPacket::Create(packetPtr->GetRawData());
    uint8_t control = paramPacket->GetControl();

    static uint8_t lastSequenceNo = 0;
    uint8_t sequenceNo = paramPacket->GetSequenceNumber();
    uint8_t lastInSeq = paramPacket->GetLastInSeq();
    std::string path = paramPacket->GetPayload()->toString();

    switch (control){
    case QuadSerialPacket::ParametersControl::SetTree:
        if((lastSequenceNo++) != sequenceNo)
        {
            logger.QuadLog(QuadGS::error, "Lost a setTree package, try again!" );
            lastSequenceNo = 0;
            RequestTree(); // We have not yet got the whole tree, continue!
            return;
        }
        if(!mTree)
        {
            size_t pos = path.find(QuadGSTree::mBranchDelimiter);
            if (pos == 0)
            {
                path.erase(0, pos + QuadGSTree::mBranchDelimiter.length());
                mTree = QuadGSTree::ptr(new QuadGSTree(path));
                mCurrentBranch = mTree;
                mTmpBranch = mTree;
                mSavedBranch = mTree;
                QuadGSTree::RemoveModuleString(path);
            }
            else
            {
                logger.QuadLog(QuadGS::error, "Packet not relative root: " + path );
                return;
            }
        }

        SetAndRegister(paramPacket->GetPayload()->toString());
        if(lastInSeq)
        {
            lastSequenceNo = 0;
        }
        else
        {
            RequestTree(); // We have not yet got the whole tree, continue!
        }
        break;
    case QuadSerialPacket::ParametersControl::GetTree:
        logger.QuadLog(QuadGS::error, "GetTree command not implemented in GS!" + path );
        break;
    case QuadSerialPacket::ParametersControl::Value:
        break;

    default:
        break;
    }
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

void Core::msgHandler(QuadGS::QuadSerialPacket::Ptr ptr)
{
    uint8_t address = ptr->GetAddress();
    switch (address){
    case QuadSerialPacket::Parameters:
        ParameterHandler(ptr);
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
    default:
        logger.QuadLog(severity_level::warning, "Unhandled message: " + std::to_string(address) + " with data: \n" + ptr->GetRawData()->toString());
        break;
    }
}




}
