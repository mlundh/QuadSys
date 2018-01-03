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

#include "AppLog.h"
namespace QuadGS {


//Forward declarations
class QGS_IoInterface;
class QGS_UiInterface;
class QGS_TrackerInterface;
class QGS_Msg;
class QGS_MsgHeader;
class QGS_DebugMsg;
class LogHandler;
class Parameters;
class QGS_UiCommand
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
    typedef std::shared_ptr < QGS_UiCommand > ptr;
    typedef std::function<std::string(std::string)> fcn;
    QGS_UiCommand(std::string name, QGS_UiCommand::fcn func, std::string doc, ActOn acton):
         mName(name)
        ,mFunc(func)
        ,mDoc(doc)
        ,mActOn(acton)
{}
    virtual ~QGS_UiCommand()
    {
    }
  std::string mName;           /* User printable name of the function. */
  fcn mFunc;                   /* Function to call to do the job. */
  std::string mDoc;            /* Documentation for this function.  */
  ActOn mActOn;
};



class QGS_CoreInterface
{
public:
    QGS_CoreInterface();

    virtual ~QGS_CoreInterface();

    /**
     * Bind method connecting a core object to an IoBase(Input/output) object.
     * @param IoPtr IoObject pointer.
     */
    virtual void bind(QGS_IoInterface* IoPtr);
    /**
     * Bind method connecting a core object to a UiBase(user interface) object.
     * @param UiPtr Pointer to an UiObject.
     */
    virtual void bind(QGS_UiInterface* UiPtr);

    /**
     * Bind method connecting a core object to a Tracker object.
     * @param TrackerPtr
     */
    virtual void bind(QGS_TrackerInterface* TrackerPtr);

    /**
     * Do all initializations that can not be done in the constructor.
     */
    virtual void initialize();

    /**
     * Function to get all commands availible to the User Interface.
     * @return  A vector of commands, with descriptions.
     */
    virtual std::vector<QGS_UiCommand::ptr> getCommands();

    /**
     * Internal write function. All writes to IO module passes this function.
     * @param  A pointer to a QSP that should be passed to the IO module.
     */
    virtual void write( std::shared_ptr<QGS_MsgHeader> header, std::shared_ptr<QGS_Msg> payload);

private:

    std::string getRuntimeStats(std::string path);

    std::string FormatRuntimeStats(std::string runtimeStats);

    /**
     * Handler function for status messages.
     * @param packetPtr Message.
     */
    virtual void StatusHandler(std::shared_ptr<QGS_MsgHeader> packetPtr);

    /**
     * Handler function for debug messages.
     * @param packetPtr Message.
     */
    virtual void DebugHandler(std::shared_ptr<QGS_MsgHeader> header, std::shared_ptr<QGS_DebugMsg> payload);

    /**
     * Message handler. Dispatches messages based on the address of the message.
     * @param ptr   Message.
     */
    virtual void msgHandler(std::shared_ptr<QGS_MsgHeader> header, std::shared_ptr<QGS_Msg> payload);

public:// TODO should be private
    std::shared_ptr<Parameters> mParameters;

private:
    AppLog logger;
    QGS_IoInterface* mIo;
    QGS_UiInterface* mUi;
    QGS_TrackerInterface* mTrack;
    std::shared_ptr<LogHandler> mLogHandler;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_CORE_CORE_H_ */
