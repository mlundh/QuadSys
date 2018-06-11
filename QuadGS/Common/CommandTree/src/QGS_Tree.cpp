/*
 * CommandBase.cpp
 *
 * Created on: May 2, 2015
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
#include <sstream>
#include <iomanip>
#include "QGS_Tree.h"

namespace QuadGS {
  
  std::string const QGS_Tree::mBranchDelimiter("/");
  std::string const QGS_Tree::mParentDelimiter("..");
  std::string const QGS_Tree::mSelfDelimiter(".");
  std::string const QGS_Tree::mValueTypeDelimiterFront("<");
  std::string const QGS_Tree::mValueTypeDelimiterBack(">");
  std::string const QGS_Tree::mValueDelimiterFront("[");
  std::string const QGS_Tree::mValueDelimiterBack("]");
  const unsigned int QGS_Tree::mMaxNameLength(8);
  const unsigned int QGS_Tree::mMaxDepth(8);
  const unsigned int QGS_Tree::mMaxExtra(5);
  const unsigned int QGS_Tree::mValueTypeLength(2);
  const unsigned int QGS_Tree::mMaxDigitsInt32(11);
  const unsigned int QGS_Tree::mMaxNodeLength(mMaxDigitsInt32 + mMaxExtra + mMaxNameLength + mValueTypeLength);

QGS_Tree::QGS_Tree(std::string name, QGS_TreeValue::NodeType_t type):
    mName(name),
    mParent(NULL),
    mValue(type),
    mLogger("Tree")
{

}

QGS_Tree::QGS_Tree(std::string path):
    mName(),
    mParent(NULL),
    mValue(QGS_TreeValue::NodeType_t::NoType)
{
    std::string module = GetModuleString(path);
    std::string name = GetModuleName(module);
    std::string valueType = GetValueTypeString(module);
    std::string value = GetValueString(module);

    mName = name;
    QGS_TreeValue::NodeType_t type = QGS_TreeValue::NodeType_t::NoType;
    if(!valueType.empty())
    {
        // Might throw
        int valueTypeI = std::stoi(valueType);
        type = static_cast<QGS_TreeValue::NodeType_t>(valueTypeI);
    }
    mValue = type;
    if(!value.empty())
    {
        mValue.SetValue(value);
    }
}

QGS_Tree::~QGS_Tree()
{

}

std::string QGS_Tree::Register(std::string path)
{
    std::string module = GetModuleString(path);
    std::string name = GetModuleName(module);
    std::string valueType = GetValueTypeString(module);
    std::string value = GetValueString(module);
    
    if(name.empty())
    {
      return "";
    }
    if(name.length() > mMaxNameLength)
    {
        throw std::runtime_error("Name: " + name + " is too long!");
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
    QGS_TreeValue::NodeType_t type = mValue.mNodeType;

    if(!valueType.empty())
    {
        // Might throw
        int valueTypeI = std::stoi(valueType);
        type = static_cast<QGS_TreeValue::NodeType_t>(valueTypeI);
    }

    ptr tmpPtr(new QGS_Tree(name, type));
    tmpPtr->SetParent(this);
    if(!value.empty())
    {
      tmpPtr->mValue.SetValue(value);
    }
    mLogger.QuadLog(severity_level::info, "registering: " + module + " on branch: " + GetName());
    mChildren.push_back(tmpPtr);
    return name;
}

std::string QGS_Tree::GetName()
{
    return mName;
}

void QGS_Tree::SetParent(QGS_Tree* parent)
{
    if(!parent)
    {
        return;
    }
    mParent = parent;
}

QGS_Tree* QGS_Tree::GetParent()
{
    return mParent;
}

std::string QGS_Tree::SetValue(const std::string path)
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

std::string QGS_Tree::GetValue()
{
    return mValue.GetValue(true);
}

std::string QGS_Tree::ModifyValue(std::string value)
{
    return mValue.ModifyValue(value);
}

QGS_Tree::ptr QGS_Tree::GetSelf()
{
  return shared_from_this();
}

QGS_Tree::ptr QGS_Tree::Find(const std::string& path)
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
        std::cout << "QuadGSTree::Find 2 returning self: " << module << " on branch: " << GetName() << std::endl;
        return GetSelf();

    }


    //We did not want to find a match (name could empty = last level) return null ptr.
    //std::cout << "QuadGSTree::Find 3 returning null due to no match"  << std::endl;
    return QGS_Tree::ptr();
}



bool QGS_Tree::NeedUpdate(const std::string& path)
{
    if(QGS_TreeValue::NodeType_t::NoType != mValue.mNodeType)
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


void QGS_Tree::FindPartial(const std::string& name, std::vector<std::string>& vec)
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



size_t QGS_Tree::getNrChildren()
{
    return mChildren.size();
}


std::string QGS_Tree::ListChildren()
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

std::string QGS_Tree::GetNodeString()
{
    std::stringstream ss;

    ss << (GetName() + "<" + std::to_string(mValue.mNodeType) + ">");
    if(QGS_TreeValue::NodeType_t::NoType != mValue.mNodeType)
    {
        ss << "[" + mValue.GetValue() + "]" << std::endl;
    }
    return ss.str();
}

bool QGS_Tree::DumpTree(std::string &tree, std::vector<size_t> &startPosition, unsigned int &depth, unsigned int length)
{
    if(depth > mMaxDepth)
    {
        throw std::runtime_error("Tree to deep, depth: " + std::to_string(depth) );

    }
    // The tree is not allowed to be longer than length.
    if((tree.length() + mMaxNodeLength) > length)
    {
        return false;
    }
    //dump self.
    tree += ("/" + GetName() + "<" + std::to_string(mValue.mNodeType) + ">");
    if(QGS_TreeValue::NodeType_t::NoType != mValue.mNodeType)
    {
        tree += "[" + mValue.GetValue() + "]";
    }
    // Dump children from start position.
    for(size_t i = startPosition[depth]; i < mChildren.size(); i++)
    {

        startPosition[depth] = i;
        depth++;
        if(mChildren[i]->DumpTree(tree, startPosition, depth, length))
        {
            tree += "/..";
        }
        else
        {
            return false;
        }
    }
    startPosition[depth] = 0;
    depth--;
    return true;
}


std::string QGS_Tree::DumpTreeFormatted(size_t depth)
{
    std::stringstream ss;
    ss.flags(std::ios::left);

    ss << std::setw(4 * static_cast<int>(depth)) << "";
    ss << std::setw(20) << (GetName() + "<" + std::to_string(mValue.mNodeType) + ">");
    if(QGS_TreeValue::NodeType_t::NoType != mValue.mNodeType)
    {
        ss << std::setw(10) << "[" + mValue.GetValue(true) + "]" << std::endl;
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

std::string QGS_Tree::GetModuleName(const std::string& s)
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

std::string QGS_Tree::GetModuleString(const std::string& s)
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

std::string QGS_Tree::GetValueTypeString(const std::string& s)
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

std::string QGS_Tree::GetValueString(const std::string& s)
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

std::string QGS_Tree::RemoveValueString(std::string& s)
{
    size_t pos_front = 0;
    size_t pos_back = 0;
    std::string token;
    pos_front = s.find(mValueDelimiterFront);
    pos_back = s.find(mValueDelimiterBack);
    if ((pos_front != std::string::npos)&&(pos_back != std::string::npos))
    {
        token = s.substr(pos_front + mValueDelimiterFront.length(), (pos_back - pos_front - mValueDelimiterBack.length()));
        s.erase(pos_front, (pos_back - pos_front +mValueDelimiterBack.length()));

    }
    return token;
}

std::string QGS_Tree::RemoveModuleString(std::string& s)
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



