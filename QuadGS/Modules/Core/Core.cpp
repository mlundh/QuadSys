/*
 * Core.cpp
 *
 *  Created on: May 14, 2015
 *      Author: martin
 */
#include "Core.h"
#include "UiBase.h"
#include "IoBase.h"
#include "QspPayloadRaw.h"
#include "QuadSerialPacket.h"


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

void Core::UpdateTmp(std::string& path, bool findFull)
{
    if(!mTree)
    {
        throw std::runtime_error("No tree registered");
    }
    size_t pos = path.find(QuadGSTree::mBranchDelimiter);
    if (pos == 0)
    {
        path.erase(0, pos + QuadGSTree::mBranchDelimiter.length());

        mTmpBranch = mTree;
        if(mTmpBranch->NeedUpdate(path))
        {
            return;
        }
        //else remove module from string and continue.
        QuadGSTree::RemoveModuleString(path);
    }
    else 
    {
        mTmpBranch = mCurrentBranch;
    }
    //std::cout << "Core::UpdateTmp 0 path: " << path << std::endl;
    //std::cout << "Core::UpdateTmp 1 Branch: " << mTmpBranch->GetName() << std::endl;
    while(!path.empty())
    {
        // Find will return shared_ptr to next element, or current if there is work to do in current node.
        QuadGSTree::ptr tmp = mTmpBranch->Find(path, findFull);
        // Node not found, return.
        if(!tmp)
        {
            return;
        }
        else
        {
            // Change to branch.
            mTmpBranch = tmp;
            // If branch need update then return.
            if(mTmpBranch->NeedUpdate(path))
            {
                return;
            }
            //else remove module from string and continue.
            QuadGSTree::RemoveModuleString(path);
            //std::cout << "Core::UpdateTmp 2 Branch: " << mTmpBranch->GetName() << std::endl;
            //std::cout << "Core::UpdateTmp 3 path: " << path << std::endl;

        }
    }
}

void Core::SaveBranch()
{
    mSavedBranch = mCurrentBranch;
}

void Core::RestoreBranch()
{
    mCurrentBranch = mSavedBranch;
}

std::string Core::ChangeBranch(std::string& path, bool findFull)
{
    UpdateTmp(path, findFull);
    // Might throw, let caller handle it.
    mCurrentBranch = mTmpBranch;
    return "";
}

std::string Core::ChangeBranchCmd(std::string path)
{
    ChangeBranch(path, true);
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
    UpdateTmp(path, true);
    std::string resp = mTmpBranch->ListChildren();
    return resp;
}


std::string Core::get(std::string path)
{
    UpdateTmp(path, true);
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
            ChangeBranch(path, true);
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


//TODO Make sure that we can call register multiple times for the same tree.
std::string Core::Register(std::string path)
{

    size_t pos = path.find(QuadGSTree::mBranchDelimiter);
    if (pos == 0)
    {
        path.erase(0, pos + QuadGSTree::mBranchDelimiter.length());
    }
    mTree = QuadGSTree::ptr(new QuadGSTree(path));
    mCurrentBranch = mTree;
    mTmpBranch = mTree;
    mSavedBranch = mTree;
    QuadGSTree::RemoveModuleString(path);

    std::exception_ptr eptr;
    SaveBranch();
    try
    {
        while(!path.empty())
        {
            ChangeBranch(path, false);
            if(!path.empty())
            {
                std::string module = mTmpBranch->Register(path);
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
    mIo->write( tmp->GetRawData() );
    return "";
}

std::string Core::writeCmd(std::string path_dump)
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
    Path += mTmpBranch->DumpTree();
    QuadSerialPacket::Ptr tmpPacket = QuadSerialPacket::Create(reinterpret_cast<const uint8_t*>(Path.c_str()),static_cast<uint16_t>(Path.length()) );
    tmpPacket->SetAddress(QuadSerialPacket::addresses::Parameters);
    tmpPacket->SetControl(QuadSerialPacket::ParametersControl::SetTree);
    mIo->write( tmpPacket->GetRawData() );
    return "";
}

std::string Core::requestUpdateCmd(std::string )
{
    QuadSerialPacket::Ptr tmpPacket = QuadSerialPacket::Create(NULL, 0 );
    tmpPacket->SetAddress(QuadSerialPacket::addresses::Parameters);
    tmpPacket->SetControl(QuadSerialPacket::ParametersControl::GetTree);
    mIo->write( tmpPacket->GetRawData() );
    return "";
}

std::string Core::saveParamCmd(std::string )
{
    QuadSerialPacket::Ptr tmpPacket = QuadSerialPacket::Create(NULL, 0 );
    tmpPacket->SetAddress(QuadSerialPacket::addresses::Parameters);
    tmpPacket->SetControl(QuadSerialPacket::ParametersControl::Save);
    mIo->write( tmpPacket->GetRawData() );
    return "";
}

std::string Core::loadParamCmd(std::string )
{
    QuadSerialPacket::Ptr tmpPacket = QuadSerialPacket::Create(NULL, 0 );
    tmpPacket->SetAddress(QuadSerialPacket::addresses::Parameters);
    tmpPacket->SetControl(QuadSerialPacket::ParametersControl::Load);
    mIo->write( tmpPacket->GetRawData() );
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
            std::bind(&Core::Register, shared_from_this(), std::placeholders::_1),
            "Register a new value in the tree.", Command::ActOn::Core));
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
    UpdateTmp(name, false);
    mTmpBranch->FindPartial(name, vec);
}

void Core::ParameterHandler(QuadSerialPacket::Ptr packetPtr)
{
    uint8_t control = packetPtr->GetControl();
    switch (control){
    case QuadSerialPacket::ParametersControl::SetTree:
        if(mTree)
        {
            set(packetPtr->GetPayload()->toString());
        }
        else
        {
            Register(packetPtr->GetPayload()->toString());
        }
        break;
    case QuadSerialPacket::ParametersControl::GetTree:
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
    case QuadSerialPacket::StatusControl::BufferOverrun:
    case QuadSerialPacket::StatusControl::Error:
    case QuadSerialPacket::StatusControl::Nack:
    case QuadSerialPacket::StatusControl::NotAllowed:
    case QuadSerialPacket::StatusControl::NotImplemented:
    case QuadSerialPacket::StatusControl::NotValidSlipPacket:
    case QuadSerialPacket::StatusControl::UnexpectedSequence:
        logger.QuadLog(QuadGS::error, "Not valid response: " + std::to_string(packetPtr->GetControl()));
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
        logger.QuadLog(severity_level::message_trace, "Parameter message: \n" + ptr->GetRawData()->toString());
        ParameterHandler(ptr);
        break;
    case QuadSerialPacket::FunctionCall:
        logger.QuadLog(severity_level::info, "FunctionCall message: \n" + ptr->GetRawData()->toString());
        // FunctionCallHandler();
        break;
    case QuadSerialPacket::Log:
        logger.QuadLog(severity_level::info, "Log message: \n" + ptr->GetRawData()->toString());
        // LogHandler();
        break;

    default:
        logger.QuadLog(severity_level::warning, "Unhandled message: " + std::to_string(address) + " with data: \n" + ptr->GetRawData()->toString());
        break;
    }
}


}
