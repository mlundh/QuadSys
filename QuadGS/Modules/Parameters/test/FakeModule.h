/*
 * FakeModule.h
 *
 *  Created on: Jun 18, 2018
 *      Author: mlundh
 */

#ifndef QUADGS_MODULES_PARAMETERS_TEST_FAKEMODULE_H_
#define QUADGS_MODULES_PARAMETERS_TEST_FAKEMODULE_H_



#include "QGS_Module.h"
#include "Msg_Param.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"
#include "Msg_FireUiCommand.h"
#include "Msg_GetUiCommands.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
using namespace std::chrono_literals;

namespace QuadGS {

class FakeModuleParam
		: public QGS_ReactiveModule
		, public QGS_MessageHandler<Msg_Param>
		, public QGS_MessageHandler<Msg_RegUiCommand>
		, public QGS_MessageHandler<Msg_UiCommandResult>

{
public:
	FakeModuleParam(msgAddr_t name):
		QGS_MessageHandlerBase(name), lastPayload(), mCommands(), lastResult(), nrResults(0), newMsg(false), mutex(), cv()
	{

	}

	virtual ~FakeModuleParam()
	{

	}

	void sendGetCommands()
	{
		Msg_GetUiCommands::ptr ptr = std::make_unique<Msg_GetUiCommands>(msgAddr_t::GS_Param_e);
		sendMsg(std::move(ptr));
	}

	void sendFireCommand(std::string command, std::string args)
	{
		Msg_FireUiCommand::ptr ptr = std::make_unique<Msg_FireUiCommand>(msgAddr_t::GS_Param_e, command, args);
		sendMsg(std::move(ptr));
	}

	void sendDummyParamMsg(uint8_t control, std::string payload)
	{
		QGS_ModuleMsgBase::ptr msg = std::make_unique<Msg_Param>(msgAddr_t::GS_Param_e, control, 0, 1, payload);
		sendMsg(std::move(msg));
	}

	std::string getLastPayload()
	{
		return lastPayload;
	}
	std::string getLastResult()
	{
		return lastResult;
	}
	int getNrResult()
	{
		return nrResults;
	}

	void waitForMsg()
	{
	    std::unique_lock<std::mutex> lk(mutex);
	    if(cv.wait_for(lk, 100ms, [&]{return newMsg;}))
	    {
	    	newMsg = false;
	    }
	}
	virtual void process(Msg_Param* message)
	{
		lastPayload = message->getPayload();
		{
			std::lock_guard<std::mutex> lk(mutex);
			newMsg = true;
		}
		cv.notify_one();
	}

	virtual void process(Msg_RegUiCommand* message)
	{
		mCommands.push_back(std::make_pair(message->getCommand(), message->getDoc()));
		{
			std::lock_guard<std::mutex> lk(mutex);
			newMsg = true;
		}
		cv.notify_one();
	}

	virtual void process(Msg_UiCommandResult* message)
	{
		lastResult = message->getResult();
		nrResults++;
		{
			std::lock_guard<std::mutex> lk(mutex);
			newMsg = true;
		}
		cv.notify_one();
	}
	std::string lastPayload;
	std::vector<std::pair<std::string, std::string>> mCommands;
	std::string lastResult;
	int nrResults;
	bool newMsg;
	std::mutex mutex;
	std::condition_variable cv;

};
}


#endif /* QUADGS_MODULES_PARAMETERS_TEST_FAKEMODULE_H_ */
