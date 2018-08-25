/*
 * FakeModule.h
 *
 *  Created on: Jun 18, 2018
 *      Author: mlundh
 */

#ifndef QUADGS_MODULES_PARAMETERS_TEST_FAKEMODULE_H_
#define QUADGS_MODULES_PARAMETERS_TEST_FAKEMODULE_H_



#include "QGS_Module.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"
#include "Msg_FireUiCommand.h"
#include "Msg_GetUiCommands.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
using namespace std::chrono_literals;

namespace QuadGS {
/**
 * Base class for testing modules with UI commands. It is important signal that a new
 * message has arrived in all new processing funcitons. This is done via the method
 * setNewMsg().
 *
 */
class FakeUiComModuleBase
		: public QGS_ReactiveModule
		, public QGS_MessageHandler<Msg_RegUiCommand>
		, public QGS_MessageHandler<Msg_UiCommandResult>

{
public:
	FakeUiComModuleBase(std::string name):
		QGS_MessageHandlerBase(name), mCommands(), lastResult(), nrResults(0), newMsg(false), mutex(), cv()
	{

	}

	virtual ~FakeUiComModuleBase()
	{

	}

	void sendGetCommands(std::string address)
	{
		Msg_GetUiCommands::ptr ptr = std::make_unique<Msg_GetUiCommands>(address);
		sendMsg(std::move(ptr));
	}

	void sendFireCommand(std::string address, std::string command, std::string args)
	{
		Msg_FireUiCommand::ptr ptr = std::make_unique<Msg_FireUiCommand>(address, command, args);
		sendMsg(std::move(ptr));
	}

	std::string getLastResult()
	{
		return lastResult;
	}
	int getNrResult()
	{
		return nrResults;
	}

	void setNewMsg()
	{
		{
			std::lock_guard<std::mutex> lk(mutex);
			newMsg = true;
		}
		cv.notify_one();
	}

	void waitForMsg()
	{
	    std::unique_lock<std::mutex> lk(mutex);
	    if(cv.wait_for(lk, 100ms, [&]{return newMsg;}))
	    {
	    	newMsg = false;
	    }
	}

	virtual void process(Msg_RegUiCommand* message)
	{
		mCommands.push_back(std::make_pair(message->getCommand(), message->getDoc()));
		setNewMsg();
	}

	virtual void process(Msg_UiCommandResult* message)
	{
		lastResult = message->getResult();
		nrResults++;
		setNewMsg();
	}
	std::vector<std::pair<std::string, std::string>> mCommands;
	std::string lastResult;
	int nrResults;
	bool newMsg;
	std::mutex mutex;
	std::condition_variable cv;

};
}


#endif /* QUADGS_MODULES_PARAMETERS_TEST_FAKEMODULE_H_ */
