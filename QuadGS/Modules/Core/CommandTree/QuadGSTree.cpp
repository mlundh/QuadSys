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
  std::string const QuadGSTree::mSelfDelimiter(".");
  std::string const QuadGSTree::mValueTypeDelimiterFront("<");
  std::string const QuadGSTree::mValueTypeDelimiterBack(">");
  std::string const QuadGSTree::mValueDelimiterFront("[");
  std::string const QuadGSTree::mValueDelimiterBack("]");


QuadGSTree::QuadGSTree(std::string name, QuadGSTreeValue::NodeType_t type):
    mName(name),
    mParent(NULL),
    mValue(type),
    mLogger("Tree")

{

}

QuadGSTree::QuadGSTree(std::string path):
    mName(),
    mParent(NULL),
    mValue(QuadGSTreeValue::NodeType_t::NoType)
{
    std::string module = GetModuleString(path);
    std::string name = GetModuleName(module);
    std::string valueType = GetValueTypeString(module);
    std::string value = GetValueString(module);

    mName = name;
    QuadGSTreeValue::NodeType_t type = QuadGSTreeValue::NodeType_t::NoType;
    if(!valueType.empty())
    {
        // Might throw
        int valueTypeI = std::stoi(valueType);
        type = static_cast<QuadGSTreeValue::NodeType_t>(valueTypeI);
    }
    mValue = type;
    if(!value.empty())
    {
        mValue.SetValue(value);
    }
}

QuadGSTree::~QuadGSTree()
{

}

std::string QuadGSTree::Register(std::string path)
{
    std::string module = GetModuleString(path);
    std::string name = GetModuleName(module);
    std::string valueType = GetValueTypeString(module);
    std::string value = GetValueString(module);
    
    if(name.empty())
    {
      return "";
    }
    // Make sure the node does not exist.
    for(unsigned int i = 0; i < mChildren.size(); i++)
    {
        std::string tmp = mChildren[i]->GetName();
        if(name.compare(tmp) == 0)
        {
            throw std::runtime_error("Branch: " + module + " already exist at current level: " + GetName());
        }
    }    
    QuadGSTreeValue::NodeType_t type = mValue.mNodeType;

    if(!valueType.empty())
    {
        // Might throw
        int valueTypeI = std::stoi(valueType);
        type = static_cast<QuadGSTreeValue::NodeType_t>(valueTypeI);
    }

    ptr tmpPtr(new QuadGSTree(name, type));
    tmpPtr->SetParent(this);
    if(!value.empty())
    {
      tmpPtr->mValue.SetValue(value);
    }
    mLogger.QuadLog(severity_level::info, "registering: " + module + " on branch: " + GetName());
    mChildren.push_back(tmpPtr);
    return name;
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

std::string QuadGSTree::SetValue(const std::string path)
{
    std::string module = GetModuleString(path);
    std::string name = GetModuleName(module);
    std::string value = GetValueString(module);
    mLogger.QuadLog(severity_level::info, "Setting: " + value + " on branch: " + GetName());

    if(value.empty())
    {
        return "No value to set.";
    }
    if(name.empty() || name.compare(GetName()) == 0 || (name.compare(mSelfDelimiter) == 0))
    {
        return mValue.SetValue(value);
    }
    return "";
}

std::string QuadGSTree::GetValue()
{
    return mValue.GetValue();
}

QuadGSTree::ptr QuadGSTree::GetSelf()
{
  return shared_from_this();
}

QuadGSTree::ptr QuadGSTree::Find(const std::string& path, bool findFull)
{
    std::string module = GetModuleString(path);
    std::string name = GetModuleName(module);
    std::string value = GetValueString(module);
    // --- Parent ---
    // If parent requested, return parent.
    if(name.compare(mParentDelimiter) == 0)
    {
      if(mParent)
      {
        return mParent->GetSelf();
      }
      throw std::runtime_error("Branch has no parent.");
    }

    // --- children ---
    for(unsigned int i = 0; i < mChildren.size(); i++)
    {

        std::string tmp = mChildren[i]->GetName();

        if(name.compare(tmp) == 0)
        {
            //std::cout << "QuadGSTree::Find 1 returning child: " << tmp << " on branch: " << GetName() << std::endl;

            return mChildren[i];
        }
    }

    // --- self ---

    // If name is this, and value is different from recorded, return self.
    if((name.compare(mSelfDelimiter) == 0)) // empty. throw?
    {
        //std::cout << "QuadGSTree::Find 2 returning self: " << module << " on branch: " << GetName() << std::endl;
        return GetSelf();

    }


    //We should have found something if anything matched.
    if(findFull)
    {
      throw std::runtime_error("Path does not exist: " + name + ".");
    }
    //If not found and we did not want to find full, or name is empty (last level) return self.
    //std::cout << "QuadGSTree::Find 3 returning null due to no match"  << std::endl;
    return QuadGSTree::ptr();
}



bool QuadGSTree::NeedUpdate(const std::string& path)
{
    if(QuadGSTreeValue::NodeType_t::NoType != mValue.mNodeType)
    {
        std::string module = GetModuleString(path);
        std::string name = GetModuleName(module);
        std::string value = GetValueString(module);
        if(value.empty())
        {
            return false;
        }
        std::string test_value = GetValue();
        bool sameValue = true;
        if(0 == value.compare(test_value))
        {
            sameValue = true;
        }
        else
        {
            sameValue = false;
        }
        //If new value and current value is not equal then return true.
        if (!sameValue )
        {
            //std::cout << "QuadGSTree::NeedUpdate 1 node not up to date: " << module << " current node: " << GetName() << std::endl;
            //std::cout << "QuadGSTree::NeedUpdate 2 value: " << value << " on branch: " << test_value << std::endl;
            return true;
        }
    }
    return false;
}


void QuadGSTree::FindPartial(const std::string& name, std::vector<std::string>& vec)
{
  std::string token = GetModuleName(name);
  //std::cout << "QuadGSTree::FindPartial 1 finding: " << token << " on branch: " << GetName() << std::endl;
  for(size_t i = 0; i < mChildren.size(); i++)
  {
      if(mChildren[i]->GetName().find(token) == 0)
      {
          //std::cout << "QuadGSTree::FindPartial 2 found: " << token << " on child: " << mChildren[i]->GetName() << std::endl;

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
          "<" + std::to_string(mChildren[i]->mValue.mNodeType) + ">" << std::endl;
    }
    std::string tmp = ss.str();
    return tmp;
}

std::string QuadGSTree::GetNodeString()
{
    std::stringstream ss;

    ss << (GetName() + "<" + std::to_string(mValue.mNodeType) + ">");
    if(QuadGSTreeValue::NodeType_t::NoType != mValue.mNodeType)
    {
        ss << "[" + mValue.GetValue() + "]" << std::endl;
    }
    return ss.str();
}

std::string QuadGSTree::DumpTree()
{
    std::string tree = ("/" + GetName() + "<" + std::to_string(mValue.mNodeType) + ">");
    if(QuadGSTreeValue::NodeType_t::NoType != mValue.mNodeType)
    {
        tree += "[" + mValue.GetValue() + "]";
    }
    for(size_t i = 0; i < mChildren.size(); i++)
    {
        tree += mChildren[i]->DumpTree() + "/..";
    }
    return tree;
}


std::string QuadGSTree::DumpTreeFormatted(size_t depth)
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
        tmp += mChildren[i]->DumpTreeFormatted( depth + 1);
    }
    return tmp;
}

std::string QuadGSTree::GetModuleName(const std::string& s)
{
    std::string module = GetModuleString(s);
    size_t pos_valueType = module.find(mValueTypeDelimiterFront);
    if(pos_valueType != std::string::npos)
    {
        module.erase(pos_valueType);
    }
    size_t pos_value = module.find(mValueDelimiterFront);
    if(pos_value != std::string::npos)
    {
        module.erase(pos_value);
    }

    return module;
}

std::string QuadGSTree::GetModuleString(const std::string& s)
{
    size_t pos = 0;
    std::string token;
    if ((pos = s.find(mBranchDelimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
    }
    else
    {
        token = s;
    }
    return token;
}

std::string QuadGSTree::GetValueTypeString(const std::string& s)
{
    size_t pos_front = 0;
    size_t pos_back = 0;
    std::string token;
    pos_front = s.find(mValueTypeDelimiterFront);
    pos_back = s.find(mValueTypeDelimiterBack);
    if ((pos_front != std::string::npos)&&(pos_back != std::string::npos))
    {
        token = s.substr(pos_front + mValueTypeDelimiterFront.length(), (pos_back - pos_front - mValueTypeDelimiterBack.length()));
    }
    return token;
}

std::string QuadGSTree::GetValueString(const std::string& s)
{
    size_t pos_front = 0;
    size_t pos_back = 0;
    std::string token;
    pos_front = s.find(mValueDelimiterFront);
    pos_back = s.find(mValueDelimiterBack);
    if ((pos_front != std::string::npos)&&(pos_back != std::string::npos))
    {
        token = s.substr(pos_front + mValueDelimiterFront.length(), (pos_back - pos_front - mValueDelimiterBack.length()));
    }
    return token;
}
std::string QuadGSTree::RemoveModuleString(std::string& s)
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

} /* namespace QuadGS */



