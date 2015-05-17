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


QuadGSTree::QuadGSTree(std::string name, QuadGSTreeValue::NodeType_t type):
    mName(name),
    mParent(NULL),
    mDelimiter("/"),
    mValueDelimiterFront("<"),
    mValueDelimiterBack(">"),
    mValue(type)

{

}

QuadGSTree::~QuadGSTree()
{

}

void QuadGSTree::Register(std::string name)
{
    std::string token = GetModuleString(name);
    std::string valueType = GetValueString(token);

    QuadGSTreeValue::NodeType_t type = mValue.mNodeType;
    if(!valueType.empty())
    {
        // Might throw
        int value = std::stoi(valueType);
        type = static_cast<QuadGSTreeValue::NodeType_t>(value);
    }

    int index = Find(token);
    if(index == -1)
    {
        mChildren.push_back(std::unique_ptr<QuadGSTree>(new QuadGSTree(token, type)));
    }
    if(name.empty())
    {
        return;
    }
    index = Find(token);
    if(index != -1)
    {
        mChildren[index]->SetParent(this);
        mChildren[index]->Register(name);
    }
    else
    {
        throw std::runtime_error("Unable to register command:" + token);
    }

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

std::string QuadGSTree::SetValue(std::string path, std::string value)
{
    if(path.empty())
    {
        return mValue.SetValue(value);
    }
    std::string token = GetModuleString(path);
    int index = Find(token);
    if(index == -1)
    {
        throw std::invalid_argument("Error: No such value (" + token + ")");
    }
    else
    {
        return(mChildren[index]->SetValue(path, value));
    }
}

std::string QuadGSTree::GetValue(std::string path)
{
    if(path.empty())
    {
        return mValue.GetValue();
    }
    std::string token = GetModuleString(path);
    int index = Find(token);
    if(index == -1)
    {
        throw std::invalid_argument("Error: No such value (" + token + ")");
    }
    else
    {
        return mChildren[index]->GetValue(path);
    }
}


int QuadGSTree::Find(std::string name)
{
    for(unsigned int i = 0; i < mChildren.size(); i++)
    {

        std::string tmp = mChildren[i]->GetName();
        if(name.compare(tmp) == 0)
        {
            return i;
        }
    }
    return -1;
}

void QuadGSTree::FindPartial(std::string& name, std::string& parent_name, std::vector<std::string>& vec)
{

    bool followPath = FollowPath(name);
    std::string token = GetModuleString(name);

    // Remove value type.
    GetValueString(token);

    if(followPath)
    {
        int index = Find(token);
        if(index != -1) // Found complete token, continue search in childrens list.
        {
            parent_name += token + mDelimiter;
            mChildren[index]->FindPartial(name, parent_name, vec);
            return;
        }
    }
    else // List at current level
    {
        for(size_t i = 0; i < mChildren.size(); i++)
        {
            if(mChildren[i]->GetName().find(token) == 0)
            {
                if(0 != mChildren[i]->getNrChildren())
                {
                    vec.push_back( parent_name + mChildren[i]->GetName() + mDelimiter);
                }
                else
                {
                    vec.push_back( parent_name + mChildren[i]->GetName());
                }

            }
        }
    }
}

size_t QuadGSTree::getNrChildren()
{
    return mChildren.size();
}

std::string QuadGSTree::DumpTree(std::string path, size_t depth)
{
     std::string token = GetModuleString(path);
     // Remove value type.
     GetValueString(token);

     int index = Find(token);
     if(index != -1) // Found complete token, do not dump self.
     {
         return mChildren[index]->DumpTree(path, depth);
     }
     // Dump self and children

    std::stringstream ss;
    ss.flags(std::ios::left);

    ss << std::setw(4 * depth) << "";
    ss << std::setw(20) << (GetName() + "<" + std::to_string(mValue.mNodeType) + ">");
    if(QuadGSTreeValue::NodeType_t::NoType != mValue.mNodeType)
    {
        ss << std::setw(10) << mValue.GetValue() << std::endl;
    }
    else
    {
        ss << std::endl;
    }

    std::string tmp = ss.str();
    for(size_t i = 0; i < mChildren.size(); i++)
    {
        tmp += mChildren[i]->DumpTree(path, depth + 1);
    }
    return tmp;
}

std::string QuadGSTree::GetModuleString(std::string& s)
{
    size_t pos = 0;
    std::string token;
    if ((pos = s.find(mDelimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        s.erase(0, pos + mDelimiter.length());
    }
    else
    {
        token = s;
        s.erase();
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
        s.erase(pos_front,  pos_back + mValueDelimiterBack.length());
    }
    return token;
}

bool QuadGSTree::FollowPath(const std::string& s)
{
    size_t pos = 0;
    if ((pos = s.find(mDelimiter)) != std::string::npos)
    {
        return true;
    }
    return false;
}

} /* namespace QuadGS */



