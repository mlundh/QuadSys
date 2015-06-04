/*
 * CommandBase.cpp
 *
 *  Created on: May 2, 2015
 *      Author: martin
 */

#include "QuadGSTree.h"
#include <sstream>
#include <iomanip>

namespace QuadGS {
  
  std::string const QuadGSTree::mBranchDelimiter("/");
  std::string const QuadGSTree::mParentDelimiter("..");
  std::string const QuadGSTree::mValueTypeDelimiterFront("<");
  std::string const QuadGSTree::mValueTypeDelimiterBack(">");
  std::string const QuadGSTree::mValueDelimiterFront("[");
  std::string const QuadGSTree::mValueDelimiterBack("]");


QuadGSTree::QuadGSTree(std::string name, QuadGSTreeValue::NodeType_t type):
    mName(name),
    mParent(NULL),
    mValue(type)

{

}

QuadGSTree::~QuadGSTree()
{

}

std::string QuadGSTree::Register(std::string name)
{
    std::string token = GetModuleString(name);
    std::string valueType = GetValueTypeString(token);
    std::string value = GetValueString(token);
    
    if(token.empty())
    {
      return "";
    }
    // Make sure the node does not exist.
    for(unsigned int i = 0; i < mChildren.size(); i++)
    {
        std::string tmp = mChildren[i]->GetName();
        if(token.compare(tmp) == 0)
        {
            throw std::runtime_error("Branch already exist.");
        }
    }    
    QuadGSTreeValue::NodeType_t type = mValue.mNodeType;
    if(!valueType.empty())
    {
        // Might throw
        int valueTypeI = std::stoi(valueType);
        type = static_cast<QuadGSTreeValue::NodeType_t>(valueTypeI);
    }
    ptr tmpPtr(new QuadGSTree(token, type));
    tmpPtr->SetParent(this);
    if(!value.empty())
    {
      tmpPtr->SetValue(value);
    }
    mChildren.push_back(tmpPtr);
    return token;
}

std::string QuadGSTree::GetName()
{
    return mName;
}

void QuadGSTree::SetParent(QuadGSTree* parent)
{
    if(!parent)
    {
        return;
    }
    mParent = parent;
}

QuadGSTree* QuadGSTree::GetParent()
{
    return mParent;
}

std::string QuadGSTree::SetValue( std::string value)
{
    return mValue.SetValue(value);
}

std::string QuadGSTree::GetValue()
{
    return mValue.GetValue();
}

QuadGSTree::ptr QuadGSTree::GetSelf()
{
  return shared_from_this();
}

QuadGSTree::ptr QuadGSTree::Find(std::string& path, bool findFull)
{
    std::string path_cpy = path;
    std::string name = GetModuleString(path_cpy);
    std::string valueType = GetValueTypeString(name);
    std::string value = GetValueString(name);
    if(name.compare(mParentDelimiter) == 0)
    {
      if(mParent)
      {
        //If found, update path and return ptr to found.
        path = path_cpy;        
        return mParent->GetSelf();
      }
      throw std::runtime_error("Branch has no parent.");
    }
    for(unsigned int i = 0; i < mChildren.size(); i++)
    {

        std::string tmp = mChildren[i]->GetName();
        if(name.compare(tmp) == 0)
        {
          //If found, update path and return ptr to found.
          path = path_cpy;          
          return mChildren[i];
        }
    }
    //If name is non empty and was not found and we wanted to find full path then throw.
    if(!name.empty() && findFull)
    {
      throw std::runtime_error("Path does not exist: " + name + ".");
    }
    //If not found and we did not want to find full, or name is empty (last level) return self, but do not update path.
    return GetSelf();
}


void QuadGSTree::FindPartial(std::string& name, std::vector<std::string>& vec)
{
  std::string token = GetModuleString(name);

  for(size_t i = 0; i < mChildren.size(); i++)
  {
      if(mChildren[i]->GetName().find(token) == 0)
      {
          vec.push_back( mChildren[i]->GetName() );
      }
  }
}

size_t QuadGSTree::getNrChildren()
{
    return mChildren.size();
}


std::string QuadGSTree::ListChildren()
{
    std::stringstream ss;
    ss.flags(std::ios::left);
    
    for(size_t i = 0; i < mChildren.size(); i++)
    {
        ss << std::setw(20) << mChildren[i]->GetName() + 
          "<" + std::to_string(mValue.mNodeType) + ">" << std::endl;
    }
    std::string tmp = ss.str();
    return tmp;
}


std::string QuadGSTree::DumpTree(size_t depth)
{
    std::stringstream ss;
    ss.flags(std::ios::left);

    ss << std::setw(4 * depth) << "";
    ss << std::setw(20) << (GetName() + "<" + std::to_string(mValue.mNodeType) + ">");
    if(QuadGSTreeValue::NodeType_t::NoType != mValue.mNodeType)
    {
        ss << std::setw(10) << "[" + mValue.GetValue() + "]" << std::endl;
    }
    else
    {
        ss << std::endl;
    }

    std::string tmp = ss.str();
    for(size_t i = 0; i < mChildren.size(); i++)
    {
        tmp += mChildren[i]->DumpTree( depth + 1);
    }
    return tmp;
}

std::string QuadGSTree::GetModuleString(std::string& s)
{
    size_t pos = 0;
    std::string token;
    if ((pos = s.find(mBranchDelimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        s.erase(0, pos + mBranchDelimiter.length());
    }
    else
    {
        token = s;
        s.erase();
    }
    return token;
}

std::string QuadGSTree::GetValueTypeString(std::string& s)
{
    size_t pos_front = 0;
    size_t pos_back = 0;
    std::string token;
    pos_front = s.find(mValueTypeDelimiterFront);
    pos_back = s.find(mValueTypeDelimiterBack);
    if ((pos_front != std::string::npos)&&(pos_back != std::string::npos))
    {

        token = s.substr(pos_front + mValueTypeDelimiterFront.length(), (pos_back - pos_front - mValueTypeDelimiterBack.length()));
        s.erase(pos_front,  (pos_back - pos_front) + mValueTypeDelimiterBack.length());
    }
    return token;
}

std::string QuadGSTree::GetValueString(std::string& s)
{
    size_t pos_front = 0;
    size_t pos_back = 0;
    std::string token;
    pos_front = s.find(mValueDelimiterFront);
    pos_back = s.find(mValueDelimiterBack);
    if ((pos_front != std::string::npos)&&(pos_back != std::string::npos))
    {

        token = s.substr(pos_front + mValueDelimiterFront.length(), (pos_back - pos_front - mValueDelimiterBack.length()));
        s.erase(pos_front,  (pos_back - pos_front) + mValueDelimiterBack.length());
    }
    return token;
}


} /* namespace QuadGS */



