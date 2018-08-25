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

namespace QuadGS {

class CLI
		: public QGS_ThreadedModule
		, public QGS_MessageHandler<Msg_RegUiCommand>
		, public QGS_MessageHandler<Msg_UiCommandResult>
		, public QGS_MessageHandler<Msg_FindParam>

{
private:


	class UiCommand
	{
	public:
		UiCommand(std::string command,  std::string doc, std::string address);
		std::string command;
		std::string doc;
		std::string address;
	};

public:
	CLI(std::string name);

	virtual ~CLI();

	void processingFcn();

	std::string stop(std::string);

	static size_t FindCommand(std::string& line);

	std::string ExecuteLine(std::string line);

	void Display(std::string str);

	void BuildPrompt();

	bool RunUI();

	void SetPrompt(std::string prompt);

	static char ** completion (const char *text, int start, int );

	static char * command_generator (const char *text, int state);

	static char * path_generator (const char *text, int state);

	static char * dupstr (const char * s);

	virtual void process(Msg_RegUiCommand* message);
	virtual void process(Msg_UiCommandResult* message);
	virtual void process(Msg_FindParam* message);

	void waitForUiResult();

public:
	std::vector<UiCommand> mCommands;
	bool mIsInitilized = false;
	std::string mPromptStatus;
	std::string mPromptBase;
	std::string mPrompt;
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



};
} /* namespace QuadGS */

#endif /* _QUADCLI_H_ */
