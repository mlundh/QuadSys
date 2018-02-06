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

#include <boost/algorithm/string.hpp>

#include "QGS_MsgHeader.h"
#include "QGS_ParamMsg.h"
#include "QGS_Module.h"
namespace QuadGS {

Parameters::Parameters():
                       logger("Parameters"),
                       mWriteFcn()
{
    mTree = QGS_Tree::ptr();
    mCurrentBranch = QGS_Tree::ptr();
    mTmpBranch = QGS_Tree::ptr();
    mSavedBranch = QGS_Tree::ptr();
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


std::vector< QGS_UiCommand::ptr > Parameters::getCommands()
{
    std::vector<std::shared_ptr < QGS_UiCommand > > mCommands;
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("cd",
            std::bind(&Parameters::ChangeBranchCmd, shared_from_this(), std::placeholders::_1),
            "Change branch", QGS_UiCommand::ActOn::Core));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("pwd",
            std::bind(&Parameters::PrintCurrentPath, shared_from_this(), std::placeholders::_1),
            "Print current branch", QGS_UiCommand::ActOn::Core));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("ls",
            std::bind(&Parameters::list, shared_from_this(), std::placeholders::_1),
            "List children on branch", QGS_UiCommand::ActOn::Core));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("paramSet",
            std::bind(&Parameters::set, shared_from_this(), std::placeholders::_1),
            "Set value of the command tree.", QGS_UiCommand::ActOn::Core));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("paramGet",
            std::bind(&Parameters::get, shared_from_this(), std::placeholders::_1),
            "Get value of the command tree.", QGS_UiCommand::ActOn::Core));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("paramAdd",
            std::bind(&Parameters::add, shared_from_this(), std::placeholders::_1),
            "Add the given value to the specified node.", QGS_UiCommand::ActOn::Core));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("paramRegisterTree",
            std::bind(&Parameters::SetAndRegister, shared_from_this(), std::placeholders::_1),
            "SetAndRegister a new value in the tree.", QGS_UiCommand::ActOn::Core));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("paramDumpTree",
            std::bind(&Parameters::dump, shared_from_this(), std::placeholders::_1),
            "Dump the command tree.", QGS_UiCommand::ActOn::Core));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("paramWrite",
            std::bind(&Parameters::writeCmd, shared_from_this(), std::placeholders::_1),
            "Write tree to port, relative to current node and path.", QGS_UiCommand::ActOn::IO));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("writeRawData",
            std::bind(&Parameters::writeRawCmd, shared_from_this(), std::placeholders::_1),
            "Write raw data to the serial port.", QGS_UiCommand::ActOn::IO));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("paramRead",
            std::bind(&Parameters::requestUpdateCmd, shared_from_this(), std::placeholders::_1),
            "Request an update from FC.", QGS_UiCommand::ActOn::IO));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("paramSave",
            std::bind(&Parameters::saveParamCmd, shared_from_this(), std::placeholders::_1),
            "Tell fc to save current parameters.", QGS_UiCommand::ActOn::IO));
    mCommands.push_back(std::make_shared<QGS_UiCommand> ("paramLoad",
            std::bind(&Parameters::loadParamCmd, shared_from_this(), std::placeholders::_1),
            "Tell fc to load saved parameters.", QGS_UiCommand::ActOn::IO));
    return mCommands;
}

void Parameters::ParameterHandler(QGS_MsgHeader::ptr header, QGSParamMsg::ptr payload)
{
    uint8_t control = header->GetControl();

    static uint8_t lastSequenceNo = 0;
    uint8_t sequenceNo = payload->GetSequenceNumber();
    uint8_t lastInSeq = payload->GetLastInSeq();
    std::string path = payload->GetPayload();

    switch (control){
    case QGS_MsgHeader::ParametersControl::SetTree:
        if((lastSequenceNo++) != sequenceNo)
        {
            logger.QuadLog(QuadGS::error, "Lost a setTree package, try again!" );
            SetAndRegister(payload->GetPayload());
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
    case QGS_MsgHeader::ParametersControl::GetTree:
        logger.QuadLog(QuadGS::error, "GetTree command not implemented in GS!" + path );
        break;
    case QGS_MsgHeader::ParametersControl::Value:
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
    size_t pos = path.find(QGS_Tree::mBranchDelimiter);
    if (pos == 0) // Update from root.
    {

        mTmpBranch = mTree;

        path.erase(0, pos + QGS_Tree::mBranchDelimiter.length());

        if(mTmpBranch->NeedUpdate(path))
        {
            return true;
        }
        //else remove module from string and continue.
        QGS_Tree::RemoveModuleString(path);
    }
    else
    {
        mTmpBranch = mCurrentBranch;
    }
    while(!path.empty())
    {
        // Find will return shared_ptr to next element, or current if there is work to do in current node.
        QGS_Tree::ptr tmp = mTmpBranch->Find(path);

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
            QGS_Tree::RemoveModuleString(path);
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
    QGS_Tree* tmp = mCurrentBranch->GetParent();
    while(tmp)
    {
        std::string parentName = tmp->GetName();
        Path = parentName + QGS_Tree::mBranchDelimiter + Path;
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
                QGS_Tree::RemoveModuleString(path);
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
    writeCmd("");
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
        size_t pos = path.find(QGS_Tree::mBranchDelimiter);
        if (pos == 0)
        {
            path.erase(0, pos + QGS_Tree::mBranchDelimiter.length());
            mTree = QGS_Tree::ptr(new QGS_Tree(path));
            mCurrentBranch = mTree;
            mTmpBranch = mTree;
            mSavedBranch = mTree;
            QGS_Tree::RemoveModuleString(path);
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
                    QGS_Tree::RemoveModuleString(path);
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

    QGS_MsgHeader::ptr header = QGS_MsgHeader::Create(iAddress, iControl, 0, 0);

    if(iAddress != QGS_MsgHeader::addresses::Parameters)
    {
        throw std::runtime_error("Only raw write of parameters are supported at the moment!");
    }

    QGSParamMsg::ptr payload = QGSParamMsg::Create(reinterpret_cast<const uint8_t*>(data.c_str()),static_cast<uint16_t>(data.length()),0 , 1);

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
	std::vector<size_t> StartPosition(QGS_Tree::mMaxDepth,0);
	uint8_t SequenceNumber = 0;
	while(cont)
	{
		UpdateTmp(path_dump);
		std::string Path;
		QGS_Tree* tmp = mTmpBranch->GetParent();
		while(tmp)
		{
			std::string parentString = tmp->GetNodeString();
			Path = QGS_Tree::mBranchDelimiter + parentString + Path;
			tmp = tmp->GetParent();
		}
		unsigned int Depth = 0;
		cont = !(mTmpBranch->DumpTree(Path, StartPosition, Depth, 256));
		Path += "/";
		QGSParamMsg::ptr payload = QGSParamMsg::Create(reinterpret_cast<const uint8_t*>(Path.c_str()),static_cast<uint16_t>(Path.length()) );
		payload->SetSequenceNumber(SequenceNumber++);

		QGS_MsgHeader::ptr header = QGS_MsgHeader::Create(QGS_MsgHeader::addresses::Parameters, QGS_MsgHeader::addresses::Parameters, false, payload->GetPayload().length());
		if(mWriteFcn)
		{
			mWriteFcn( header, std::static_pointer_cast<QGS_Msg>(payload) );
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
    QGS_MsgHeader::ptr header = QGS_MsgHeader::Create(QGS_MsgHeader::addresses::Parameters,
            QGS_MsgHeader::ParametersControl::Save,0,0);
    std::shared_ptr<QGS_Msg> nullPtr;
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
    QGS_MsgHeader::ptr header = QGS_MsgHeader::Create(QGS_MsgHeader::addresses::Parameters,
            QGS_MsgHeader::ParametersControl::Load,0,0);
    std::shared_ptr<QGS_Msg> nullPtr;
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
    QGS_MsgHeader::ptr tmpPacket = QGS_MsgHeader::Create(QGS_MsgHeader::addresses::Parameters,
            QGS_MsgHeader::ParametersControl::GetTree,0,0);
    std::shared_ptr<QGS_Msg> nullPtr;
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
