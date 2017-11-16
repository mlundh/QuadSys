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
#include "Log.h"
namespace QuadGS {


//Forward declarations
class IoBase;
class UiBase;
class QuadGSMsg;
class QCMsgHeader;
class QuadDebugMsg;
class LogHandler;
class Parameters;
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
{
public:
    virtual ~Core();

    /**
     * Static create method. Use this to create an instance of Core.
     * @return  Pointer to instance of core.
     */
    static Core* create();

    /**
     * Bind method connecting a core object to an IoBase(Input/output) object.
     * @param IoPtr IoObject pointer.
     */
    virtual void bind(IoBase* IoPtr);
    /**
     * Bind method connecting a core object to a UiBase(user interface) object.
     * @param UiPtr Pointer to an UiObject.
     */
    virtual void bind(UiBase* UiPtr);

    /**
     * Function to get all commands availible to the User Interface.
     * @return  A vector of commands, with descriptions.
     */
    virtual std::vector<Command::ptr> getCommands();

    /**
     * Internal write function. All writes to IO module passes this function.
     * @param  A pointer to a QSP that should be passed to the IO module.
     */
    virtual void write( std::shared_ptr<QCMsgHeader> header, std::shared_ptr<QuadGSMsg> payload);

private:

    std::string getRuntimeStats(std::string path);

    std::string FormatRuntimeStats(std::string runtimeStats);

    /**
     * Handler function for status messages.
     * @param packetPtr Message.
     */
    virtual void StatusHandler(std::shared_ptr<QCMsgHeader> packetPtr);

    /**
     * Handler function for debug messages.
     * @param packetPtr Message.
     */
    virtual void DebugHandler(std::shared_ptr<QCMsgHeader> header, std::shared_ptr<QuadDebugMsg> payload);

    /**
     * Message handler. Dispatches messages based on the address of the message.
     * @param ptr   Message.
     */
    virtual void msgHandler(std::shared_ptr<QCMsgHeader> header, std::shared_ptr<QuadGSMsg> payload);

public:// TODO should be private
    std::shared_ptr<Parameters> mParameters;

private:
    Core();
    Log logger;
    IoBase* mIo;
    UiBase* mUi;
    std::shared_ptr<LogHandler> mLogHandler;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_CORE_H_ */
