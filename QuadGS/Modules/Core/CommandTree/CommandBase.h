/*
 * CommandBase.h
 *
 *  Created on: Apr 26, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_CORE_COMMANDTREE_COMMANDBASE_H_
#define QUADGS_MODULES_CORE_COMMANDTREE_COMMANDBASE_H_


#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "CommandValue.h"

namespace QuadGS {

class CommandBase
{
public:

    typedef std::shared_ptr<CommandBase> ptr;

    CommandBase(std::string name, CommandValue::NodeType_t type = CommandValue::NodeType_t::NoType);

    virtual ~CommandBase();

    void Register(std::string name);

    std::string GetName();

    void SetParent(CommandBase* parent);

    CommandBase* GetParent();

    virtual bool SetValue(std::string path, std::string value);

    virtual bool GetValue(std::string path, std::string& value);

    int Find(std::string name);

protected:

    std::string mName;
    CommandBase* mParent;
    const std::string mDelimiter;
    const std::string mValueDelimiterFront;
    const std::string mValueDelimiterBack;
    std::vector< CommandBase::ptr > mChildren;
    CommandValue mValue;

    std::string GetModuleString(std::string& s);
    std::string GetValueString(std::string& s);
};


} /* namespace QuadGS */


#endif /* QUADGS_MODULES_CORE_COMMANDTREE_COMMANDBASE_H_ */
