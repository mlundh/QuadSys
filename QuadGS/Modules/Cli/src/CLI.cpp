/*
 * QuadCLI.cpp
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

#include "CLI.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>
#include <boost/algorithm/string.hpp>
#include <chrono>

#include "Msg_Stop.h"

namespace QuadGS {

char  CLI::WordBreakPath[] = " \t\n\"\\'`@$><=;|&{(/";
char  CLI::WordBreak[]     = " \t\n\"\\'`@$><=;|&{(";
CLI* CLI::cli = NULL;

CLI::UiCommand::UiCommand(std::string command, std::string args, msgAddr_t address):
						command(command), doc(args), address(address)
{

}

CLI::CLI(msgAddr_t name)
:QGS_MessageHandlerBase(name)
,mCommands()
,mPromptStatus("N/A")
,mPromptBase("QuadGS")
,mPrompt()
{
	cli = this;
	rl_attempted_completion_function = completion;
	read_history(NULL);
	mCommands.push_back(UiCommand("quit","",getName()));
	setProcessingFcn(std::bind(&CLI::processingFcn, this));
	startProcessing();
}

CLI::~CLI()
{
	cli = NULL;
	write_history(NULL);
	history_truncate_file(NULL, 100);
}

void CLI::processingFcn()
{
	handleMessages(true);
}

std::string CLI::stop(std::string)
{
	stopProcessing();
	return "";
}


size_t CLI::FindCommand(std::string& line)
{
	if(line.empty())
	{
		throw std::runtime_error("Empty line.");
	}

	std::istringstream iss(line);
	std::string command;
	iss >> command;
	line.erase(line.begin(), line.begin() + command.size());
	boost::trim(line);

	for(size_t i = 0; i < cli->mCommands.size(); i++)
	{
		if(!cli->mCommands[i].command.find(command))
		{
			return i;
		}
	}
	throw std::runtime_error("No such command.");
}

std::string CLI::ExecuteLine(std::string line)
{
	size_t i = FindCommand(line); // FindCommands leaves only args in line.
	mLogger.QuadLog(debug, "Calling command: " + mCommands[i].command);
	if(mCommands[i].address == getName() && mCommands[i].command == "quit")
	{
		stopProcessing();
		return"";
	}
	// Send and wait for reply...
	Msg_FireUiCommand::ptr ptr = std::make_unique<Msg_FireUiCommand>(mCommands[i].address, mCommands[i].command, line);
	sendMsg(std::move(ptr));
	waitForUiResult();
	return "";
}

void CLI::Display(std::string str)
{
	std::cout << std::endl << str << std::endl;
}

void CLI::BuildPrompt()
{
	if(mPromptStatus.empty())
	{
		mPrompt = mPromptBase + ": ";
	}
	else
	{
		mPrompt = mPromptBase + "-[" + mPromptStatus + "]" + ": ";
	}
}


bool CLI::RunUI()
{
	if(!mIsInitilized)
	{
		mLogger.QuadLog(debug, "Requesting messages from GS_Param");

		Msg_GetUiCommands::ptr ptr = std::make_unique<Msg_GetUiCommands>(msgAddr_t::GS_Param_e);
		sendMsg(std::move(ptr));
		Msg_GetUiCommands::ptr ptrLh = std::make_unique<Msg_GetUiCommands>(msgAddr_t::GS_Log_e);
		sendMsg(std::move(ptrLh));
		mIsInitilized = true;
	}
	try
	{
		char * tmp_line;
		BuildPrompt();
		tmp_line = readline (mPrompt.c_str());
		std::string line(tmp_line);
		free(tmp_line);

		boost::trim(line);
		if (line.empty())
			return true;

		add_history (line.c_str());
		std::string resp = ExecuteLine(line);
		if(!resp.empty())
		{
			std::cout << resp << std::endl;
		}

	}
	catch(const std::invalid_argument& e)
	{
		mLogger.QuadLog(error, e.what());
		return true;
	}
	catch(const std::runtime_error& e)
	{
		mLogger.QuadLog(error, e.what());
		return true;
	}

	catch(const std::exception& e)
	{
		mLogger.QuadLog(error, e.what());
		return true;
	}
	return !mStop;
}




/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
char ** CLI::completion (const char *text, int start, int)
{

	char **matches;

	matches = (char **)NULL;

	/* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is a path in the tree. */
	if (start == 0)
	{
		rl_completer_word_break_characters = WordBreak;
		matches = rl_completion_matches (text, command_generator);
	}
	else
	{
		std::string tmpLine(rl_line_buffer);
		size_t i = 0;
		try
		{
			i = FindCommand(tmpLine);
		}
		catch(const std::runtime_error& e)
		{
			return (matches);
		}

		if(cli->mCommands[i].address == msgAddr_t::GS_Param_e)
		{
			rl_completer_word_break_characters = WordBreakPath;
			rl_attempted_completion_over = 1;

			matches = rl_completion_matches (tmpLine.c_str(), path_generator);

			rl_completer_word_break_characters = WordBreak;

		}
		else if(cli->mCommands[i].address != msgAddr_t::GS_Param_e)
		{

		}

	}
	return (matches);
}

/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. rl will free the returned char *, therefore
   we have to  malloc the c_str.*/
char * CLI::command_generator (const char *text, int state)
{
	static unsigned int list_index, len;

	/* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
	if (!state)
	{
		list_index = 0;
		len = static_cast<unsigned int>(strlen (text));
	}

	/* Return the next name which partially matches from the command list. */
	while(list_index < cli->mCommands.size())
	{
		list_index++;
		if (strncmp (cli->mCommands[list_index - 1].command.c_str(), text, len) == 0)
			return (dupstr(cli->mCommands[list_index - 1].command.c_str()));
	}

	/* If no names matched, then return NULL. */
	return ((char *)NULL);
}

/* Generator function for path completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. rl will free the returned char *, therefore
   we have to  malloc the c_str.*/
char * CLI::path_generator (const char *text, int state)
{
	rl_completion_append_character = '/';
	static unsigned int list_index;
	static std::vector<std::string> vec;


	/* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
	std::string text_s(text);

	if (!state)
	{
		list_index = 0;
		vec.clear();
		Msg_FindParam::ptr ptr = std::make_unique<Msg_FindParam>(msgAddr_t::GS_Param_e, text_s, "");
		cli->sendMsg(std::move(ptr));

		// Now wait for message to get back
		std::unique_lock<std::mutex> l(cli->mMutex);

		if(!cli->cvFindParam.wait_for(l, std::chrono::milliseconds(5), [](){return cli->newFindParam; }))
		{
			throw std::runtime_error("Did not get a find param result");
		}
		cli->newUiRsp = false;

		text_s = cli->mFindParamToFind;

		// Msg_FindParam returns a comma separated list, we should convert this to a vector of strings.
		while(!cli->mFindParamResult.empty())
		{
			size_t pos = 0;
			std::string token;
			if ((pos = cli->mFindParamResult.find(",")) != std::string::npos)
			{
				token = cli->mFindParamResult.substr(0, pos);
				cli->mFindParamResult.erase(0, pos + 1);
				vec.push_back(token);
			}
			else
			{
				token = cli->mFindParamResult;
				cli->mFindParamResult.clear();
				vec.push_back(token);
			}
		}
	}
	boost::trim(text_s);

	/* Return the next name which partially matches from the command list. */
	if(!vec.empty())
	{
		while(list_index < vec.size())
		{
			list_index++;
			if(0 == vec[list_index-1].find(text_s))
			{
				return dupstr(vec[list_index-1].c_str());
			}
		}
	}
	/* If no names matched, then return NULL. */
	return ((char *)NULL);
}


char * CLI::dupstr (const char * s)
{
	char *r;

	r = (char*)malloc((unsigned long int)(strlen (s) + 1));
	strcpy (r, s);
	return (r);
}

void CLI::process(Msg_RegUiCommand* message)
{
	mLogger.QuadLog(debug, "Received UI command: " + message->getCommand() + " from " + msgAddrStr[message->getSource()]);
	mCommands.push_back(UiCommand(message->getCommand(), message->getDoc(), message->getSource()));
}
void CLI::process(Msg_UiCommandResult* message)
{
	std::lock_guard<std::mutex> lock(mMutex);

	newUiRsp = true;
	Display(message->getResult());

	mMutex.unlock();
	cvNewUiRsp.notify_one();
}
void CLI::process(Msg_FindParam* message)
{
	std::lock_guard<std::mutex> lock(mMutex);

	mFindParamResult = message->getFound();
	mFindParamToFind = message->getTofind();
	newFindParam = true;

	mMutex.unlock();
	cvFindParam.notify_one();

}

void CLI::waitForUiResult()
{
	std::unique_lock<std::mutex> l(mMutex);
	if(!cvNewUiRsp.wait_for(l, std::chrono::milliseconds(5), [this](){return newUiRsp; }))
	{
		throw std::runtime_error("Did not get a ui command result");
	}
	newUiRsp = false;
}

} /* namespace QuadGS */

