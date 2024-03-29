/*
 * QuadCLI.h
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

#ifndef _QUADCLI_H_
#define _QUADCLI_H_
#include <string>
#include <vector>

#include "QGS_Module.h"
#include <mutex>
#include <condition_variable>
#include "Msg_GetUiCommands.h"
#include "Msg_RegUiCommand.h"
#include "Msg_FireUiCommand.h"
#include "Msg_UiCommandResult.h"
#include "Msg_FindParam.h"
#include "Msg_Stop.h"
#include "Msg_Display.h"
#include "Msg_FlightMode.h"
#include "Msg_CtrlMode.h"
#include "msg_enums.h"


namespace QuadGS {

class CLI
		: public QGS_ThreadedModule
		, public QGS_MessageHandler<Msg_RegUiCommand>
		, public QGS_MessageHandler<Msg_UiCommandResult>
		, public QGS_MessageHandler<Msg_FindParam>
		, public QGS_MessageHandler<Msg_Display>
		, public QGS_MessageHandler<Msg_FlightMode>
		, public QGS_MessageHandler<Msg_CtrlMode>


{
private:

	std::map<CtrlMode_t, std::string> mapCtrlModeToString =
	{
		{ CtrlMode_t::Control_mode_rate, "Rate" },
		{ CtrlMode_t::Control_mode_attitude, "Attitude" },
		{ CtrlMode_t::Control_mode_not_available, "N/A" },
	};

	std::map<FlightMode_t, std::string> mapFlightModeToString =
	{
		{ FlightMode_t::fmode_init, "init" },
		{ FlightMode_t::fmode_disarmed, "disarmed" },
		{ FlightMode_t::fmode_config, "config" },
		{ FlightMode_t::fmode_arming, "arming" },
		{ FlightMode_t::fmode_armed, "armed"},
		{ FlightMode_t::fmode_disarming,"disarm" },
		{ FlightMode_t::fmode_fault, "fault" },
		{ FlightMode_t::fmode_not_available, "Not availible"},
	};
	class UiCommand
	{
	public:
		UiCommand(std::string command,  std::string doc, msgAddr_t address);
		std::string command;
		std::string doc;
		msgAddr_t address;
	};

public:
	CLI(msgAddr_t name);

	virtual ~CLI();

	void processingFcn();

	std::string stop(std::string);

	static size_t FindCommand(std::string& line);

	static std::string ExecuteLine(std::string line);

	static void Display(std::string str);

	void BuildPrompt();

	bool RunUI();

	static void ProcessLine(char *line);

	static char *my_rl_complete(char *token, int *match);

	static int my_rl_list_possib(char *token, char ***av);

	static std::string path_generator (const char *text, int state, bool removeBase = false);

	static char * dupstr (const char * s);

	virtual void process(Msg_RegUiCommand* message);
	virtual void process(Msg_UiCommandResult* message);
	virtual void process(Msg_FindParam* message);
	virtual void process(Msg_Display* message);
	virtual void process(Msg_FlightMode* message);
	virtual void process(Msg_CtrlMode* message);

	void waitForUiResult();

public:
	std::vector<UiCommand> mCommands;
	bool mIsInitilized = false;
	std::string mPromptStatus;
	std::string mPromptCtrlMode;
	std::string mPromptPath;
	std::string mPromptBase;
	std::string mPrompt;
	bool updatePrompt = false;
	static char WordBreakPath[];
	static char WordBreak[];
	static CLI* cli;

	std::mutex mMutex;

	std::string mFindParamResult;
	std::string mFindParamToFind;

	bool newFindParam = false; // c++11, fancy! :)
	std::condition_variable cvFindParam;

	bool newUiRsp = false;
	std::condition_variable cvNewUiRsp;
	std::string uiRsp;

	static bool stopCli;



};
} /* namespace QuadGS */

#endif /* _QUADCLI_H_ */
