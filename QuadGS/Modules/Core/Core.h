/*
 * Core.h
 *
 *  Created on: May 10, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_CORE_CORE_H_
#define QUADGS_MODULES_CORE_CORE_H_
#include "QuadGSTree.h"
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "Log.h"
namespace QuadGS {





class Command : public Log
{
public:
    typedef std::shared_ptr < Command > ptr;
    typedef std::function<std::string(std::string)> fcn;
    Command(std::string name, Command::fcn func, std::string doc):
        Log("Command")
        ,mName(name)
        ,mFunc(func)
        ,mDoc(doc)
{}
    virtual ~Command()
    {
        QuadLog(severity_level::debug, "Destructor of Command " + mName + " called" );
    }
  std::string mName;           /* User printable name of the function. */
  fcn mFunc;                   /* Function to call to do the job. */
  std::string mDoc;            /* Documentation for this function.  */
};



class Core
        : public std::enable_shared_from_this<Core>
        , public Log
{
public:
    typedef std::shared_ptr<Core> ptr;
    virtual ~Core();
    static ptr create();
    std::string get(std::string path);
    std::string set(std::string path);
    std::string Register(std::string path);
    std::string dump(std::string path);
    std::vector<Command::ptr> getCommands();
    QuadGSTree::ptr getTree();

private:
    Core();

    QuadGSTree::ptr mTree = QuadGSTree::ptr(new QuadGS::QuadGSTree("QuadFC"));
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_CORE_H_ */
