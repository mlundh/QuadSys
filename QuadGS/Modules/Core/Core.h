/*
 * Core.h
 *
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

    /**
     * Static create method. Use this to create an instance of Core.
     * @return  Pointer to instance of core.
     */
    static ptr create();

    /**
     * Bind method connecting a core object to an IoBase(Input/output) object.
     * @param IoPtr IoObject pointer.
     */
    void bind(std::shared_ptr<IoBase> IoPtr);
    /**
     * Bind method connecting a core object to a UiBase(user interface) object.
     * @param UiPtr Pointer to an UiObject.
     */
    void bind(std::shared_ptr<UiBase> UiPtr);

    /**
     * Update the temp branch to point at path. mTmpBranch will point at
     * the node selected by path, or the first node in the chain that needs to be
     * updated to correspond to the path. All nodes in path before the function
     * returns will be removed.
     * @param path      Path to the node.
     * @return          return true if path was found, false otherwise.
     */
    bool UpdateTmp(std::string& path);

    /**
     * Save current branch. Current branch can be restored by a call to RestoreBranch()
     */
    void SaveBranch();

    /**
     * Restore the previously saved branch.
     */
    void RestoreBranch();

    /**
     * Change branch to path. If path includes nodes that need to be updated, then
     * the method will only change to the first of the branches that needs an update.
     * @param path
     * @return
     */
    bool ChangeBranch(std::string& path);

    /**
     * User commands that can be used by the UI.
     */
    std::string ChangeBranchCmd(std::string path);
    std::string PrintCurrentPath(std::string pathIn);
    std::string list(std::string path);
    std::string get(std::string path);
    std::string set(std::string path);
    std::string SetAndRegister(std::string path);
    std::string writeRawCmd(std::string data);
    std::string writeCmd(std::string path_dump);
    std::string requestUpdateCmd(std::string path_dump);
    std::string saveParamCmd(std::string );
    std::string loadParamCmd(std::string );
    std::string dump(std::string path);

    /**
     * Function to get all commands availible to the User Interface.
     * @return  A vector of commands, with descriptions.
     */
    std::vector<Command::ptr> getCommands();

    /**
     * Get a pointer to this instance.
     * @return  Shared pointer to this.
     */
    ptr getThis();

    /**
     * Find all nodes in the tree that matches the string "name".
     * @param name  incomplete name to search for.
     * @param vec   Vector of matches.
     */
    void FindPartial(std::string& name, std::vector<std::string>& vec);

    /**
     * Internal write function. All writes to IO module passes this function.
     * @param  A pointer to a QSP that should be passed to the IO module.
     */
    void write( std::shared_ptr<QuadSerialPacket>);

    /**
     * Internal get tree function. Will request next param message.
     */
    void RequestTree();

    /**
     * Handler function for parameter messages.
     * @param packetPtr Message.
     */
    void ParameterHandler(std::shared_ptr<QuadSerialPacket> packetPtr);

    /**
     * Handler function for status messages.
     * @param packetPtr Message.
     */
    void StatusHandler(std::shared_ptr<QuadSerialPacket> packetPtr);

    /**
     * Message handler. Dispatches messages based on the address of the message.
     * @param ptr   Message.
     */
    void msgHandler(std::shared_ptr<QuadSerialPacket> ptr);

    
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
