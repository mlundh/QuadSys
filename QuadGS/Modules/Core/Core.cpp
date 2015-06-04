/*
 * Core.cpp
 *
 *  Created on: May 14, 2015
 *      Author: martin
 */
#include "Core.h"
#include <boost/algorithm/string.hpp>
namespace QuadGS {

Core::Core():
         Log("Core")
{
  mTree = QuadGSTree::ptr(new QuadGS::QuadGSTree("QuadFC"));
  mCurrentBranch = mTree;
  mTmpBranch = mTree;
  mSavedBranch = mTree;
}

Core::~Core()
{
  
}

Core::ptr Core::create()
{
    ptr tmp = ptr(new Core);
    return tmp;
}

void Core::UpdateTmp(std::string& path, bool findFull)
{
  size_t pos = path.find(QuadGSTree::mBranchDelimiter);
  if (pos == 0)
  {
    path.erase(0, pos + QuadGSTree::mBranchDelimiter.length());
    mTmpBranch = mTree;    
  }
  else 
  {
    mTmpBranch = mCurrentBranch;    
  }
  bool foundLast = false;
  while(!foundLast)
  {
    // Find will return shared_ptr to next element, or current if no match to path is found.
    
    QuadGSTree::ptr tmp = mTmpBranch->Find(path, findFull);
    
    if(tmp == mTmpBranch)
    {
      foundLast = true;
      // if path is none empty then we had faulty string. Reset tmpBranch and throw if we are expecting a full path.
      if(!path.empty() && findFull)
      {
        mTmpBranch = mCurrentBranch;
        throw std::runtime_error("No such path in tree.");
      }
    }
    else 
    {
      mTmpBranch = tmp;
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
  std::string Path = mCurrentBranch->GetName();
  QuadGSTree* tmp = mCurrentBranch->GetParent();
  while(tmp)
  {
    std::string parentName = tmp->GetName();
    Path = parentName + QuadGSTree::mBranchDelimiter + Path;
    tmp = tmp->GetParent();
  }

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
    // Might throw, let caller handle it.
    std::string value = mTmpBranch->GetValue();
    return value;
}
std::string Core::set(std::string pathAndValue)
{
    // path should have one and only one space, first path then value.
    std::istringstream iss(pathAndValue);
    std::string path;

    iss >> path;
    pathAndValue.erase(pathAndValue.begin(), pathAndValue.begin() + path.size());
    boost::trim(pathAndValue);

    std::string value = pathAndValue;
    
    UpdateTmp(path, true);
    
    value = mTmpBranch->SetValue(value);
    return value;
}

std::string Core::Register(std::string path)
{
  std::exception_ptr eptr;
  SaveBranch();
  try
  {
    while(!path.empty())
    {      
      ChangeBranch(path, false);      
      std::string module = mTmpBranch->Register(path);
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

std::string Core::dump(std::string path)
{
    UpdateTmp(path);
    return mTmpBranch->DumpTree(0);
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
    mCommands.push_back(std::make_shared<Command> ("setValueTree",
            std::bind(&Core::set, shared_from_this(), std::placeholders::_1),
            "Set value of the command tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("getValueTree",
            std::bind(&Core::get, shared_from_this(), std::placeholders::_1),
            "Get value of the command tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("registerValueTree",
            std::bind(&Core::Register, shared_from_this(), std::placeholders::_1),
            "Register a new value in the tree.", Command::ActOn::Core));
    mCommands.push_back(std::make_shared<Command> ("dumpTree",
            std::bind(&Core::dump, shared_from_this(), std::placeholders::_1),
            "Dump the command tree.", Command::ActOn::Core));

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


}
