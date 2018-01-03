/*
 * Parameters.h
 *
 * Copyright (C) 2016 Martin Lundh
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

#ifndef QUADGS_MODULES_CORE_PARAMETERS_PARAMETERS_H_
#define QUADGS_MODULES_CORE_PARAMETERS_PARAMETERS_H_

#include <memory>
#include <functional>
#include <string>
#include <vector>

#include "AppLog.h"
#include "QGS_Tree.h"
#include "QGS_ParamMsg.h"

namespace QuadGS {
class QGS_UiCommand;
class QGS_MsgHeader;

class Parameters
        : public std::enable_shared_from_this<Parameters>
{
    typedef std::function<void(std::shared_ptr<QGS_MsgHeader>, std::shared_ptr<QGS_Msg>) > WriteFcn;

    /**
     * Default contstructor is private, we are using shared pointers instead.
     */
    Parameters();

public:
    typedef std::shared_ptr<Parameters> ptr;

    virtual ~Parameters();

    static ptr create();

    /**
     * Register the function that will write the packet to the client.
     * @param fcn
     */
    void RegisterWriteFcn(WriteFcn fcn);

    /**
     * Function to get all commands availible to the User Interface.
     * @return  A vector of commands, with descriptions.
     */
    std::vector<std::shared_ptr<QGS_UiCommand> > getCommands();

    /**
     * Handler function for parameter messages.
     * @param packetPtr Message.
     */
    void ParameterHandler(std::shared_ptr<QGS_MsgHeader> header, std::shared_ptr<QGSParamMsg> payload);

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
    std::string add(std::string path);
    std::string SetAndRegister(std::string path);
    std::string writeRawCmd(std::string data);
    std::string writeCmd(std::string path_dump);
    std::string requestUpdateCmd(std::string path_dump);
    std::string saveParamCmd(std::string );
    std::string loadParamCmd(std::string );
    std::string dump(std::string path);


    /**
     * Find all nodes in the tree that matches the string "name".
     * @param name  incomplete name to search for.
     * @param vec   Vector of matches.
     */
    void FindPartial(std::string& name, std::vector<std::string>& vec);

    /**
     * Internal get tree function. Will request next param message.
     */
    void RequestTree();


    AppLog logger;
    WriteFcn mWriteFcn;
    QGS_Tree::ptr mTmpBranch;
    QGS_Tree::ptr mCurrentBranch;
    QGS_Tree::ptr mSavedBranch;
    QGS_Tree::ptr mTree;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_PARAMETERS_PARAMETERS_H_ */
