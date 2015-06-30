/*
 * Core.h
 *
 *  Created on: May 10, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_CORE_CORE_H_
#define QUADGS_MODULES_CORE_CORE_H_
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "QuadGSTree.h"
#include "Log.h"
namespace QuadGS {


//Forward declarations
class IoBase;
class UiBase;
class QspPayloadRaw;
class QuadSerialPacket;

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
{
public:
    typedef std::shared_ptr<Core> ptr;
    virtual ~Core();
    static ptr create();
    void bind(std::shared_ptr<IoBase> IoPtr);
    void bind(std::shared_ptr<UiBase> UiPtr);
    void UpdateTmp(std::string& path, bool findFull = true);
    void SaveBranch();
    void RestoreBranch();
    std::string ChangeBranch(std::string& path, bool findFull);
    std::string ChangeBranchCmd(std::string path);
    std::string PrintCurrentPath(std::string pathIn);
    std::string list(std::string path);
    std::string get(std::string path);
    std::string set(std::string path);
    std::string Register(std::string path);
    std::string writeRawCmd(std::string data);
    std::string writeCmd(std::string path_dump);
    std::string dump(std::string path);
    std::vector<Command::ptr> getCommands();
    ptr getThis();
    void FindPartial(std::string& name, std::vector<std::string>& vec);
    void ParameterHandler(std::shared_ptr<QuadSerialPacket> packetPtr);
    void msgHandler(std::shared_ptr<QspPayloadRaw> ptr);

    
private:
    Core();
    Log logger;
    QuadGSTree::ptr mTmpBranch;
    QuadGSTree::ptr mCurrentBranch;
    QuadGSTree::ptr mSavedBranch;
    QuadGSTree::ptr mTree;
    std::shared_ptr<IoBase> mIo;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_CORE_H_ */
