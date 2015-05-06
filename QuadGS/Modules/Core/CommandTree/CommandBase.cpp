/*
 * CommandBase.cpp
 *
 *  Created on: May 2, 2015
 *      Author: martin
 */

#include "CommandBase.h"

namespace QuadGS {


CommandBase::CommandBase(std::string name, CommandValue::NodeType_t type):
    mName(name),
    mParent(NULL),
    mDelimiter("/"),
    mValueDelimiterFront("<"),
    mValueDelimiterBack(">"),
    mValue(type)

{

}

CommandBase::~CommandBase()
{

}

void CommandBase::Register(std::string name)
{
    std::string token = GetModuleString(name);
    //std::cout << "1:-name of this: " << mName << std::endl;
    //std::cout << "2:-ValueType of this: " << mValue.mNodeType << std::endl;
    //std::cout << "3:--token: " << token << std::endl;
    std::string valueType = GetValueString(token);
    //std::cout << "4:--ValueType from string: " << valueType << std::endl;
    //std::cout << "5:---token after value type extraction: " << token << std::endl;

    CommandValue::NodeType_t type = mValue.mNodeType;
    if(!valueType.empty())
    {
        try
        {
            int value = std::stoi(valueType);
            type = static_cast<CommandValue::NodeType_t>(value);
        }
        catch (std::invalid_argument& e)
        {
            return;
        }
        catch (std::out_of_range& e)
        {
            return;
        }

    }

    int index = Find(token);
    if(index == -1)
    {
        mChildren.push_back(std::make_shared<CommandBase>(token, type));
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
        //todo error, should be found, we just put it there!
    }

}

std::string CommandBase::GetName()
{
    return mName;
}

void CommandBase::SetParent(CommandBase* parent)
{
    if(!parent)
    {
        return;
    }
    mParent = parent;
}

CommandBase* CommandBase::GetParent()
{
    return mParent;
}

bool CommandBase::SetValue(std::string path, std::string value)
{
    if(path.empty())
    {
        mValue.SetValue(value);
        return true;
    }
    std::string token = GetModuleString(path);
    int index = Find(token);
    if(index == -1)
    {
        return false;
    }
    else
    {
        return(mChildren[index]->SetValue(path, value));
    }
}

bool CommandBase::GetValue(std::string path, std::string& value)
{
    if(path.empty())
    {
        mValue.GetValue(value);
        return true;
    }
    std::string token = GetModuleString(path);
    int index = Find(token);
    if(index == -1)
    {
        return -1; // TODO error!!
    }
    else
    {
        return mChildren[index]->GetValue(path, value);
    }
}


int CommandBase::Find(std::string name)
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


std::string CommandBase::GetModuleString(std::string& s)
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

std::string CommandBase::GetValueString(std::string& s)
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



} /* namespace QuadGS */



