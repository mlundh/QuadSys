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

#include <functional>
#include <sstream> 
#include "msg_enums.h"
#include "msgAddr.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"

#include "QGS_Module.h"

using namespace std::placeholders;

namespace QuadGS {

Parameters::Parameters(msgAddr_t name, msgAddr_t send_name):QGS_MessageHandlerBase(name), mSendName(send_name), sequenceNr(0)

{
	mRoot = QGS_Tree::ptr(new QGS_Tree(""));
	mCurrentBranch = mRoot;
	mTmpBranch = QGS_Tree::ptr();
	mCommands.push_back(UiCommand("paramCd","Change working branch",std::bind(&Parameters::ChangeBranchCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramPrintBranch","Print current branch",std::bind(&Parameters::PrintCurrentPath, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramList","List children on branch",std::bind(&Parameters::list, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramSet","Set value of the command tree. [parameter] [value]",std::bind(&Parameters::set, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramGet","Get value of the command tree. [parameter]",std::bind(&Parameters::get, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramAdd","Add the given value to the specified node. [parameter] [value]",std::bind(&Parameters::add, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramRegisterTree","SetAndRegister a new value in the tree.[parameter tree]",std::bind(&Parameters::SetAndRegister, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramDumpTree","Dump the command tree.",std::bind(&Parameters::dump, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramWrite","Write tree to port, relative to current node and path.[parameter]",std::bind(&Parameters::writeCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramRead","Request an update from FC.",std::bind(&Parameters::requestUpdateCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramSave","Tell fc to save current parameters.",std::bind(&Parameters::saveParamCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("paramLoad","Tell fc to load saved parameters.",std::bind(&Parameters::loadParamCmd, this, std::placeholders::_1)));
}

Parameters::~Parameters()
{
}

bool Parameters::UpdateTmp(std::string& path, bool restart)
{
	if(restart)
	{
		mTmpBranch = mCurrentBranch;
	}
	if(path.empty())
	{
		mTmpBranch = mCurrentBranch;
		return true;
	}
	size_t pos = path.find(QGS_Tree::mBranchDelimiter);
	if (pos == 0) // Update from root.
	{
		mTmpBranch = mRoot;
		path.erase(0, pos + QGS_Tree::mBranchDelimiter.length());
	}
	if(!mCurrentBranch)
	{
		mCurrentBranch = mRoot;// Fallback...
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
	if(!mRoot->getNrChildren())
	{
		throw std::runtime_error("No tree registered");
	}
	std::string Path = mCurrentBranch->GetName();
	QGS_Tree* tmp = mCurrentBranch->GetParent();
	while(tmp)
	{
		if(tmp->GetName() != "")
		{
			std::string parentName = tmp->GetName();
			Path = parentName + QGS_Tree::mBranchDelimiter + Path;	
		}
		
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
	std::string result;
	std::exception_ptr eptr;
	try
	{
		while(!path.empty())
		{
			bool found = UpdateTmp(path,false);
			if(!found)
			{
				result = "No such path.";
				break;
			}
			else if(!path.empty())
			{
				mTmpBranch->SetValue(path);
				QGS_Tree::RemoveModuleString(path);
			}
		}
	}
	catch(...)
	{
		eptr = std::current_exception();
		std::rethrow_exception(eptr);
	}
	writeCmd("");
	return result;
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
	std::exception_ptr eptr;
	// Update tmp and then register a new child, or set the correct value. Do this untill the 
	// path is empty. 
 	try
	{
		while(!path.empty())
		{
			bool found = UpdateTmp(path, false);
			if(!path.empty() && (path[0] == 0)) // Remove nulltermination of string if present.
			{
				path.erase();
			}
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
		std::rethrow_exception(eptr);
	}
	return "";
}



std::string Parameters::writeCmd(std::string path_dump)
{
	UpdateTmp(path_dump, false);
	writeInternal(path_dump, true);

	return "";
}

void Parameters::writeInternal(std::string path_dump, bool restart)
{
	bool cont = true;
	std::vector<size_t> StartPosition(QGS_Tree::mMaxDepth,0);
	uint8_t SequenceNumber = 0;
	while(cont)
	{
		UpdateTmp(path_dump, restart);
		std::string Path;
		QGS_Tree* tmp = mTmpBranch->GetParent();
		while(tmp) 
		{
			if(tmp->GetName() != "")
			{
				std::string parentString = tmp->GetNodeString();
				Path = QGS_Tree::mBranchDelimiter + parentString + Path;
			}
			
			tmp = tmp->GetParent();
		}
		unsigned int Depth = 0;
		cont = !(mTmpBranch->DumpTree(Path, StartPosition, Depth, 256));
		Path += "/";

		Msg_Param::ptr ptr = std::make_unique<Msg_Param>(mSendName, ParamCtrl::param_set, SequenceNumber++, 0, Path);
		//mLogger.QuadLog(QuadGS::error, ptr->getPayload());

		sendMsg(std::move(ptr));
	}
}

std::string Parameters::requestUpdateCmd(std::string )
{
	if(sequenceNr != 0)
	{
		return "Ongoing.";
	}
	RequestTree();
	return "";
}

std::string Parameters::saveParamCmd(std::string )
{
	Msg_Param::ptr ptr = std::make_unique<Msg_Param>(mSendName, ParamCtrl::param_save,0,0,"");
	sendMsg(std::move(ptr));
	return "";
}

std::string Parameters::loadParamCmd(std::string )
{
	Msg_Param::ptr ptr = std::make_unique<Msg_Param>(mSendName, ParamCtrl::param_load,0,0,"");
	sendMsg(std::move(ptr));
	return "";
}

std::string Parameters::dump(std::string path)
{
	std::stringstream result;

	UpdateTmp(path);
	result << mTmpBranch->DumpTreeFormatted(0);	
	
	return  result.str();

}

void Parameters::FindPartial(std::string& name, std::vector<std::string>& vec)
{
	std::string basePath(name);
	UpdateTmp(name);
	uint lengthBase = basePath.size();
	uint lengthPath = name.size();
	basePath.erase(lengthBase - lengthPath, lengthPath);
	if(lengthPath == 0 && lengthBase != 0 && (lengthBase != lengthPath) && (basePath.back() != '/'))
	{
		basePath.append("/");
	}
	mTmpBranch->FindPartial(name, vec, basePath);
}



void Parameters::RequestTree()
{
	Msg_Param::ptr ptr = std::make_unique<Msg_Param>(mSendName, ParamCtrl::param_get,0,0,"");
	std::cout << "Request tree" << std::endl;
	sendMsg(std::move(ptr));
}


void Parameters::process(Msg_Param* message)
{
	LOG_MESSAGE_TRACE(log, "Processing incoming param message.");

	uint8_t control = message->getControl();

	uint8_t receivedNr = message->getSequencenr();
	uint8_t lastInSeq = message->getLastinsequence();
	std::string path = message->getPayload();
	
	switch (control){
	case ParamCtrl::param_set:
		if(sequenceNr != receivedNr)
		{
			std::stringstream ss;
			ss << "Lost a setTree package! Got: " << (int)receivedNr << " expected: " << (int) sequenceNr;
			LOG_ERROR(log, ss.str());
			sequenceNr = 0;
			break;
		}
		SetAndRegister(message->getPayload());
		if(lastInSeq)
		{
			std::cout << "last in sequence!" << std::endl;
			sequenceNr = 0;
		}
		else
		{
			sequenceNr++;
			RequestTree(); // We have not yet got the whole tree, continue!
		}
		break;
	case ParamCtrl::param_get:
		LOG_ERROR(log, "GetTree command not implemented in GS!" + path );
		break;
	default:
		break;
	}
}

void Parameters::process(Msg_GetUiCommands* message)
{
	for(UiCommand command : mCommands)
	{
		Msg_RegUiCommand::ptr ptr = std::make_unique<Msg_RegUiCommand>(message->getSource(), command.command, command.doc);
		sendMsg(std::move(ptr));
	}
}
void Parameters::process(Msg_FireUiCommand* message)
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

void Parameters::process(Msg_FindParam* message)
{
	std::vector<std::string> vec;
	std::string toFind = message->getTofind();
	FindPartial(toFind, vec);
	std::string result;
	// serialize the result, find param message uses comma separated results.
	for(auto item : vec)
	{
		result = result + item + ",";
	}
	Msg_FindParam::ptr ptr = std::make_unique<Msg_FindParam>(message->getSource(),toFind, result);
	sendMsg(std::move(ptr));
}


}
/* namespace QuadGS */
