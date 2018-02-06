/*
 * QGSModule.h
 *
 *  Copyright (C) 2018 Martin Lundh
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


#ifndef QUADGS_UTILITIES_QGS_MODULE_QGS_MODULE_H_
#define QUADGS_UTILITIES_QGS_MODULE_QGS_MODULE_H_


#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include "AppLog.h"
#include "ThreadSafeFifo.hpp"
#include "QGS_ModuleMsg.h"

namespace QuadGS
{
class QGS_Router;
struct QGS_UiCommand
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
	std::string mName;           /* Human readable name of the function. */
	fcn mFunc;                   /* Function to call to do the job. */
	std::string mDoc;            /* Documentation for this function.  */
	ActOn mActOn;
};

struct QGS_UiCommand_
{
public:
	typedef std::shared_ptr < QGS_UiCommand > ptr;
	typedef std::function<std::string(std::string)> fcn;
	QGS_UiCommand_(std::string name, QGS_UiCommand::fcn func, std::string doc):
		mName(name)
	,mFunc(func)
	,mDoc(doc)
	{}
	virtual ~QGS_UiCommand_()
	{
	}
	std::string mName;           /* Human readable name of the function. */
	fcn mFunc;                   /* Function to call to do the job. */
	std::string mDoc;            /* Documentation for this function.  */
};

/**
 * @class QGS_Module
 * @brief base class for all modules connected to a QGS_Router
 *
 * The basic QGS_Module is not threaded, and is only reactive - it is not the source of any actions,
 * it only reacts to incoming messages and responds (or not depending on the message). Users have to
 * implement processMsg and registerCommands. The basic QGS_Module is executed in the thread that sends
 * a message, normally this would be the router thread.
 */
class QGS_Module: virtual public QGS_MessageHandlerBase
{
public:
    friend QGS_Router;
	typedef std::function<void(std::shared_ptr<QGS_ModuleMsg>) > receivingFcn_t;

	QGS_Module(std::string name);

	virtual ~QGS_Module();

	/**
	 * Bind the Module to a Router. This enables the module to talk to other modules
	 * bound to the same router. Messages are distributed based on type and subscriptions.
	 * @param router
	 */
	void bind(QGS_Router* router);

	/**
	 * Get the name of the module. This name will be included in all logs from the module.
	 * @return
	 */
	std::string& getName();

protected:


	/**
	 * Register the custom UI commands the module can handle
	 */
	virtual void registerCommands() = 0;

	/**
	 * Subscribe to a particular message type. Call multiple times to subscribe to multiple message types.
	 * @param type
	 */
	void subscribeMsg(messageTypes_t type);

	/**
	 * Set the method that receives the messages.
	 */
	void setReceivingFcn(receivingFcn_t fcn);

	/**
	 * Get the receiving port of the module. This is used for binding.
	 * @return
	 */
	WriteFcn getReceivingFcn();

	/**
	 * Send messages. This function can be used by the module in the processMsg implementation to
	 * send a message.
	 * @param message
	 */
	void sendMsg(std::shared_ptr<QGS_ModuleMsg> message);

	/**
	 * Utility function used internally to handle incoming messages of type getCommands.
	 * @return QGS_Msg shared pointer containing information of all commands.
	 */
	std::shared_ptr<QGS_ModuleMsg> getCommands();

	/**
	 * Utility function used internally to handle incoming messages of type executeCommands.
	 * @param command
	 * @param args
	 * @return
	 */
	std::string executeCommand(std::string command, std::string args);

	/**
	 * Used internally during binding. Sets the send function and enables the module to
	 * send messages to the router (and through the router to other modules).
	 * @param func
	 */
	void setSendFunc(WriteFcn func);


	WriteFcn mReceiveFcn;
    WriteFcn mSendFcn;
    std::string mName;
    std::vector<QGS_UiCommand_> mCommands;
};


class QGS_ReactiveModule: public QGS_Module
{
public:
	typedef std::function<void() > processingFcn;

	QGS_ReactiveModule(std::string name);

	virtual ~QGS_ReactiveModule();

private:

	void ReceivingFcn(std::shared_ptr<QGS_ModuleMsg> message);
};



/**
 * @class QGS_ThreadedModule
 * @brief Base class for all active modules connected to a QGS_Router.
 *
 * This type of QGS_Module is threaded, and can thus be both active and reactive. All thread sync and thread
 * safety is handled by this base class.
 */
class QGS_ThreadedModule: public QGS_Module
{
public:
	typedef std::function<void() > processingFcn;

	QGS_ThreadedModule(std::string name);

	virtual ~QGS_ThreadedModule();

protected:

	/**
	 * Start the thread. Call this at the end of the derived class constructor, or at any time after that.
	 */
	void startProcessing();

	/**
	 *
	 */
	void stopProcessing();

	/**
	 * Set the function that will implement the modules functionality.
	 *
	 * This method will be called repeatedly as long as the threaded module exists. All functionality
	 * should occur in this method. To process messages the handleMessages method must be called from
	 * this method.
	 */
	void setProcessingFcn(processingFcn fcn);

	/**
	 * Call this method repeatedly to handle new messages.
	 */
	void handleMessages(bool blocking);

private:

	void ReceivingFcn(std::shared_ptr<QGS_ModuleMsg> message);

	void runThread();

private:
    bool mStop;
	std::thread mThread;
	ThreadSafeFifo<QGS_ModuleMsg::ptr> mFifo; // ,essage, originating port
	processingFcn mProcessingFcn;

};

} /* namespace QuadGS */

#endif /* QUADGS_UTILITIES_QGS_MODULE_QGS_MODULE_H_ */
