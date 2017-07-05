/*
 * Parameters.cpp
 *
 * Copyright (C) 2016 Martin Lundh
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

#include "Parameters.h"
#include "Core.h"
#include <boost/algorithm/string.hpp>

#include "DataMsg/QuadParamPacket.h"
#include "DataMsg/QCMsgHeader.h"

namespace QuadGS {

Parameters::Parameters():
        		        logger("Parameters"),
        		        mWriteFcn(NULL)
{
    mTree = QuadGSTree::ptr();
    mCurrentBranch = QuadGSTree::ptr();
    mTmpBranch = QuadGSTree::ptr();
    mSavedBranch = QuadGSTree::ptr();
}

Parameters::~Parameters()
{
    // TODO Auto-generated destructor stub
}

Parameters::ptr Parameters::create()
{
    ptr tmp = ptr(new Parameters);
    return tmp;
}

void Parameters::RegisterWriteFcn(WriteFcn fcn)
{
    mWriteFcn = fcn;
}


std::vector< Command::ptr > Parameters::getCommands()
{
    std::vector<std::shared_ptr < Command > > mCommands;
    mCommands.push_back(std::make_shared<Command> ("cd",
            std::bind(&Parameters::ChangeBranchCmd, shared_from_this(), std::placeholders::_1),
            "Change branch", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("pwd",
            std::bind(&Parameters::PrintCurrentPath, shared_from_this(), std::placeholders::_1),
            "Print current branch", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("ls",
            std::bind(&Parameters::list, shared_from_this(), std::placeholders::_1),
            "List children on branch", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("set",
            std::bind(&Parameters::set, shared_from_this(), std::placeholders::_1),
            "Set value of the command tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("get",
            std::bind(&Parameters::get, shared_from_this(), std::placeholders::_1),
            "Get value of the command tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("add",
            std::bind(&Parameters::add, shared_from_this(), std::placeholders::_1),
            "Add the given value to the specified node.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("RegisterTree",
            std::bind(&Parameters::SetAndRegister, shared_from_this(), std::placeholders::_1),
            "SetAndRegister a new value in the tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("dumpTree",
            std::bind(&Parameters::dump, shared_from_this(), std::placeholders::_1),
            "Dump the command tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("write",
            std::bind(&Parameters::writeCmd, shared_from_this(), std::placeholders::_1),
            "Write tree to port, relative to current node and path.", Command::ActOn::IO));
    mCommands.push_back(std::make_shared<Command> ("writeRawData",
            std::bind(&Parameters::writeRawCmd, shared_from_this(), std::placeholders::_1),
            "Write raw data to the serial port.", Command::ActOn::IO));
    mCommands.push_back(std::make_shared<Command> ("read",
            std::bind(&Parameters::requestUpdateCmd, shared_from_this(), std::placeholders::_1),
            "Request an update from FC.", Command::ActOn::IO));
    mCommands.push_back(std::make_shared<Command> ("save",
            std::bind(&Parameters::saveParamCmd, shared_from_this(), std::placeholders::_1),
            "Tell fc to save current parameters.", Command::ActOn::IO));
    mCommands.push_back(std::make_shared<Command> ("load",
            std::bind(&Parameters::loadParamCmd, shared_from_this(), std::placeholders::_1),
            "Tell fc to load saved parameters.", Command::ActOn::IO));
    return mCommands;
}

void Parameters::ParameterHandler(QCMsgHeader::ptr header, QuadParamPacket::ptr payload)
{
    uint8_t control = header->GetControl();

    static uint8_t lastSequenceNo = 0;
    uint8_t sequenceNo = payload->GetSequenceNumber();
    uint8_t lastInSeq = payload->GetLastInSeq();
    std::string path = payload->GetPayload();

    switch (control){
    case QCMsgHeader::ParametersControl::SetTree:
        if((lastSequenceNo++) != sequenceNo)
        {
            logger.QuadLog(QuadGS::error, "Lost a setTree package, try again!" );
            lastSequenceNo = 0;
            RequestTree(); // We have not yet got the whole tree, continue!
            return;
        }

        SetAndRegister(payload->GetPayload());
        if(lastInSeq)
        {
            lastSequenceNo = 0;
        }
        else
        {
            RequestTree(); // We have not yet got the whole tree, continue!
        }
        break;
    case QCMsgHeader::ParametersControl::GetTree:
        logger.QuadLog(QuadGS::error, "GetTree command not implemented in GS!" + path );
        break;
    case QCMsgHeader::ParametersControl::Value:
        break;

    default:
        break;
    }
}

bool Parameters::UpdateTmp(std::string& path)
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

void Parameters::SaveBranch()
{
    mSavedBranch = mCurrentBranch;
}

void Parameters::RestoreBranch()
{
    mCurrentBranch = mSavedBranch;
}

bool Parameters::ChangeBranch(std::string& path)
{
    bool found = UpdateTmp(path);
    mCurrentBranch = mTmpBranch;
    return found;
}

std::string Parameters::ChangeBranchCmd(std::string path)
{
    ChangeBranch(path);
    if(!path.empty())
    {
        throw std::runtime_error("Path does not exist: " + path + ".");
    }
    return "";
}

std::string Parameters::PrintCurrentPath(std::string)
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

std::string Parameters::list(std::string path)
{
    UpdateTmp(path);
    if(!path.empty())
    {
        throw std::runtime_error("Path does not exist: " + path + ".");
    }
    std::string resp = mTmpBranch->ListChildren();
    return resp;
}


std::string Parameters::get(std::string path)
{
    UpdateTmp(path);
    if(!path.empty())
    {
        throw std::runtime_error("Get command can not contain set information.");
    }
    std::string value = mTmpBranch->GetValue();
    return value;
}
std::string Parameters::set(std::string path)
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

std::string Parameters::add(std::string path)
{
    std::vector<std::string> args;
    boost::split( args, path, boost::is_any_of(" ") );


    if(args.size() != 2)
    {
        throw std::runtime_error("inc takes two parameters!");
    }
    UpdateTmp(args[0]);

    std::string value = mTmpBranch->ModifyValue(args[1]);

    writeCmd(args[0]);
    return value;
}


std::string Parameters::SetAndRegister(std::string path)
{
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
            return "";
        }
    }
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


std::string Parameters::writeRawCmd(std::string data)
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

    QCMsgHeader::ptr header = QCMsgHeader::Create(iAddress, iControl, 0, 0);

    if(iAddress != QCMsgHeader::addresses::Parameters)
    {
        throw std::runtime_error("Only raw write of parameters are supported at the moment!");
    }

    QuadParamPacket::ptr payload = QuadParamPacket::Create(reinterpret_cast<const uint8_t*>(data.c_str()),static_cast<uint16_t>(data.length()),0 , 1);

    if(mWriteFcn)
    {
        mWriteFcn( header, payload );
    }
    else
    {
        logger.QuadLog(QuadGS::error, "No write function registered!");
    }
    return "";
}

std::string Parameters::writeCmd(std::string path_dump)
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
        QuadParamPacket::ptr payload = QuadParamPacket::Create(reinterpret_cast<const uint8_t*>(Path.c_str()),
                static_cast<uint16_t>(Path.length()),
                0,
                0);
        payload->SetSequenceNumber(SequenceNumber++);
        uint16_t length = payload->GetPayload().length()+1;		// +1 since there is an extra byte of data in the beginning of a param payload.
        QCMsgHeader::ptr header = QCMsgHeader::Create(QCMsgHeader::addresses::Parameters, QCMsgHeader::addresses::Parameters, false, length);
        if(mWriteFcn)
        {
            mWriteFcn( header, std::static_pointer_cast<QuadGSMsg>(payload) );
        }
        else
        {
            logger.QuadLog(QuadGS::error, "No write function registered!");
        }
    }
    return "";
}

std::string Parameters::requestUpdateCmd(std::string )
{
    RequestTree();
    return "";
}

std::string Parameters::saveParamCmd(std::string )
{
    QCMsgHeader::ptr header = QCMsgHeader::Create(QCMsgHeader::addresses::Parameters,
            QCMsgHeader::ParametersControl::Save,0,0);
    std::shared_ptr<QuadGSMsg> nullPtr;
    if(mWriteFcn)
    {
        mWriteFcn( header,  nullPtr);
    }
    else
    {
        logger.QuadLog(QuadGS::error, "No write function registered!");
    }
    return "";
}

std::string Parameters::loadParamCmd(std::string )
{
    QCMsgHeader::ptr header = QCMsgHeader::Create(QCMsgHeader::addresses::Parameters,
            QCMsgHeader::ParametersControl::Load,0,0);
    std::shared_ptr<QuadGSMsg> nullPtr;
    if(mWriteFcn)
    {
        mWriteFcn( header,  nullPtr);
    }
    else
    {
        logger.QuadLog(QuadGS::error, "No write function registered!");
    }
    return "";
}

std::string Parameters::dump(std::string path)
{
    UpdateTmp(path);
    return  mTmpBranch->DumpTreeFormatted(0);

}

void Parameters::FindPartial(std::string& name, std::vector<std::string>& vec)
{
    UpdateTmp(name);
    mTmpBranch->FindPartial(name, vec);
}



void Parameters::RequestTree()
{
    QCMsgHeader::ptr tmpPacket = QCMsgHeader::Create(QCMsgHeader::addresses::Parameters,
            QCMsgHeader::ParametersControl::GetTree,0,0);
    std::shared_ptr<QuadGSMsg> nullPtr;
    if(mWriteFcn)
    {
        mWriteFcn( tmpPacket,  nullPtr);
    }
    else
    {
        logger.QuadLog(QuadGS::error, "No write function registered!");
    }
}

}
/* namespace QuadGS */
