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





class Command
{
public:
  enum ActOn
  {
    NoAction,
    Core,
    IO,
    UI,
    File
  };
    typedef std::shared_ptr < Command > ptr;
    typedef std::function<std::string(std::string)> fcn;
    Command(std::string name, Command::fcn func, std::string doc, ActOn acton):
         mName(name)
        ,mFunc(func)
        ,mDoc(doc)
        ,mActOn(acton)
{}
    virtual ~Command()
    {
    }
  std::string mName;           /* User printable name of the function. */
  fcn mFunc;                   /* Function to call to do the job. */
  std::string mDoc;            /* Documentation for this function.  */
  ActOn mActOn;
};



class Core
        : public std::enable_shared_from_this<Core>
        , public Log
{
public:
    typedef std::shared_ptr<Core> ptr;
    virtual ~Core();
    static ptr create();
    void UpdateTmp(std::string& path, bool findFull = true);
    void SaveBranch();
    void RestoreBranch();
    std::string ChangeBranch(std::string& path, bool findFull);
    std::string ChangeBranchCmd(std::string path);
    std::string PrintCurrentPath(std::string path);
    std::string list(std::string path);
    std::string get(std::string path);
    std::string set(std::string path);
    std::string Register(std::string path);
    std::string dump(std::string path);
    std::vector<Command::ptr> getCommands();
    ptr getThis();
    void FindPartial(std::string& name, std::vector<std::string>& vec);
    
private:
    Core();
    
    QuadGSTree::ptr mTmpBranch;
    QuadGSTree::ptr mCurrentBranch;
    QuadGSTree::ptr mSavedBranch;
    QuadGSTree::ptr mTree;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_CORE_H_ */
