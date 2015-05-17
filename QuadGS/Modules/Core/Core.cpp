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

}
Core::~Core()
{
    QuadLog(severity_level::debug, "Destructor called" );
}

Core::ptr Core::create()
{
    ptr tmp = ptr(new Core);
    return tmp;
}

std::string Core::get(std::string path)
{
    std::string value;
    // Might throw, let caller handle it.
    value = mTree->GetValue(path);
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

    // Might throw, let caller handle it.
    value = mTree->SetValue(path, value);
    return value;
}

std::string Core::Register(std::string path)
{
    std::string value;
    // Might throw, let caller handle it.
    mTree->Register(path);
    return "Registered: " + path;
}

std::string Core::dump(std::string path)
{
    return mTree->DumpTree(path, 0);
}
std::vector< Command::ptr > Core::getCommands()
{
    std::vector<std::shared_ptr < Command > > mCommands;

    mCommands.push_back(std::make_shared<Command> ("set",
            std::bind(&Core::set, shared_from_this(), std::placeholders::_1),
            "Set value of the command tree."));
    mCommands.push_back(std::make_shared<Command> ("get",
            std::bind(&Core::get, shared_from_this(), std::placeholders::_1),
            "Get value of the command tree."));
    mCommands.push_back(std::make_shared<Command> ("register",
            std::bind(&Core::Register, shared_from_this(), std::placeholders::_1),
            "Register a new value in the tree."));
    mCommands.push_back(std::make_shared<Command> ("dump",
            std::bind(&Core::dump, shared_from_this(), std::placeholders::_1),
            "Dump the command tree."));

    return mCommands;
}

QuadGSTree::ptr Core::getTree()
{
    return mTree;
}

}
