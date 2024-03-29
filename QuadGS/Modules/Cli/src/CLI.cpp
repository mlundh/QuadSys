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

#include <editline.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>
#include <boost/algorithm/string.hpp>
#include <chrono>

#include "Msg_Stop.h"
#include "msgAddr.h"

namespace QuadGS {

char  CLI::WordBreakPath[] = " \t\n\"\\'`@$><=;|&{(/";
char  CLI::WordBreak[]     = " \t\n\"\\'`@$><=;|&{(";
CLI* CLI::cli = NULL;
bool CLI::stopCli = false;
CLI::UiCommand::UiCommand(std::string command, std::string args, msgAddr_t address):
						command(command), doc(args), address(address)
{

}
// There are many thread-saftey issues here. The main thread handles the editline, but the worker-thread
// handles the module. These two would neeed to synchronize much better.
CLI::CLI(msgAddr_t name)
:QGS_MessageHandlerBase(name)
,mCommands()
,mPromptStatus("N/A")
,mPromptCtrlMode("N/A")
,mPromptPath("")
,mPromptBase("QuadGS")
,mPrompt()
{
	cli = this;
	BuildPrompt();
	el_hist_size = 1000;
	read_history("QuadGS.hist");
	rl_set_complete_func(&my_rl_complete);
	rl_set_list_possib_func(&my_rl_list_possib);

	mCommands.push_back(UiCommand("quit","",getName()));
	setProcessingFcn(std::bind(&CLI::processingFcn, this));
	startProcessing();

}

CLI::~CLI()
{
	cli = NULL;
	write_history("QuadGS.hist");	
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
		if((cli->mCommands[i].command.size() == command.size()) && !cli->mCommands[i].command.find(command))
		{

			return i;
		}
	}
	throw std::runtime_error("No such command.");
}

std::string CLI::ExecuteLine(std::string line)
{
	size_t i;
	try
	{
		i = FindCommand(line); // FindCommands leaves only args in line.
	}
	catch(const std::runtime_error& e)
	{
		return "No such command!";
	}
	
	LOG_DEBUG(cli->log, "Calling command: " << cli->mCommands[i].command);
	if(cli->mCommands[i].address == cli->getName() && cli->mCommands[i].command == "quit")
	{
		cli->mStop = true;
		cli->stopProcessing();
		return"";
	}
	// Send and wait for reply...
	Msg_FireUiCommand::ptr ptr = std::make_unique<Msg_FireUiCommand>(cli->mCommands[i].address, cli->mCommands[i].command, line);
	cli->sendMsg(std::move(ptr));
	cli->waitForUiResult();
	return cli->uiRsp;
}

void CLI::Display(std::string str)
{
	char line_buf[256];
	strncpy(line_buf, rl_line_buffer, sizeof(line_buf));
  	line_buf[sizeof(line_buf) - 1] = 0;
	if(!str.empty())
	{
		std::cout << std::endl << str << std::endl;
	}
}

void CLI::BuildPrompt()
{
	mPrompt = mPromptBase + "-[" + mPromptStatus + "][" + mPromptCtrlMode + "]" + ": ";
	updatePrompt = true;
}


bool CLI::RunUI()
{
	if(!mIsInitilized)
	{
		Msg_GetUiCommands::ptr ptr = std::make_unique<Msg_GetUiCommands>(msgAddr_t::GS_Broadcast_e);
		sendMsg(std::move(ptr));

		mIsInitilized = true;
	}
	while(!mStop)
	{
		char *line;
		line = readline(mPrompt.c_str());
		ProcessLine(line);
	}
	Msg_Stop::ptr ptr = std::make_unique<Msg_Stop>(msgAddr_t::GS_Broadcast_e);
	sendMsg(std::move(ptr));
	return !mStop;
}

void CLI::ProcessLine(char *line)
{
	try
	{
		std::string strLine(line);
		free(line);

		boost::trim(strLine);
		if (strLine.empty())
			return;

		add_history (strLine.c_str());
		std::string resp = ExecuteLine(strLine);
		if(!resp.empty())
		{
			Display(resp);
		}

	}
	catch(const std::invalid_argument& e)
	{
		//mLogger.QuadLog(error, e.what());
		return;
	}
	catch(const std::runtime_error& e)
	{
		//mLogger.QuadLog(error, e.what());
		return;
	}

	catch(const std::exception& e)
	{
		//mLogger.QuadLog(error, e.what());
		return;
	}
	return;
}

/* Attempt to complete the pathname, returning an allocated copy.
 * Fill in *unique if we completed it, or set it to 0 if ambiguous. */
char *CLI::my_rl_complete(char *token, int *match)
{
	bool firstWord = (strlen(token) >= strlen(rl_line_buffer));
	uint len = static_cast<unsigned int>(strlen (token));

	if(firstWord)
	{
		/* Return the number of characters that is common between all partial matches from the command list. */
		std::vector<uint> matches;

		for( uint i = 0; i < cli->mCommands.size(); i++)
		{
			if (strncmp (cli->mCommands[i].command.c_str(), token, len) == 0)
			{
				matches.push_back(i);
			}
		}
		uint nrMatchChar = 255;
		if(matches.size() == 1) // Complete the whole word if there is only one match.
		{
			*match = 1;
			return (strdup(cli->mCommands[matches[0]].command.c_str() + len));
		}
		else if(matches.size() > 1) // Complete the longest common string among all matches.
		{
    	    for(uint index : matches)
			{
				uint i = 0;
    	        for(i = len; (i < cli->mCommands[index].command.size()) && 
							    (i < cli->mCommands[matches[0]].command.size()); i++)
				{
					if(cli->mCommands[index].command[i] != cli->mCommands[matches[0]].command[i])
					{
						break;
					}
    	        }
				if(i < (nrMatchChar))
				{
					nrMatchChar = i;
				}
    	    }
			if(nrMatchChar > len) // Return the common string only if it adds anything to one on the line already.
			{
				*match = 1;
				return (strndup(cli->mCommands[matches[0]].command.c_str() + len, nrMatchChar - len));
			}
		}
	}
	else
	{
		std::string tmpLine(rl_line_buffer);

		size_t i;
		try
		{
			i = FindCommand(tmpLine); // FindCommands leaves only args in line.
		}
		catch(const std::runtime_error& e)
		{
			return NULL;
		}
		if(cli->mCommands[i].address == msgAddr_t::GS_Param_e)
		{	
			int nbr = 255;
			std::string firstEntry = path_generator(token, 0);
			if(firstEntry.empty())
			{
				return NULL;
			}
			else
			{
				nbr = firstEntry.length();
			}
			bool cont = true;
			while (cont)
			{
				std::string entry = path_generator(token, 1);
				int j = 0;
				if(!entry.empty())
				{
					while(firstEntry[j] == entry[j])
					{
						j++;
					}
					if(j < nbr)
					{
						nbr = j;
					}
				}
				else
				{
					cont = false;
				}
			}
			int tokenLength = strlen(token);
			char* result = NULL;
			if(nbr > tokenLength)
			{
				result = strndup(firstEntry.c_str() + tokenLength, nbr - tokenLength);
			}
			return result;

		}
		else
		{
			char* result = NULL;
			rl_set_complete_func(NULL);
			result = rl_complete(token, match);
			rl_set_complete_func(&my_rl_complete);
			return result;
		}
	}
	
	/* If no names matched, then return NULL. */
    return NULL;
}

/* Return all possible completions. */
int CLI::my_rl_list_possib(char *token, char ***av)
{
	bool firstWord = (strlen(token) >= strlen(rl_line_buffer));

	int total = 0;

	if(firstWord)
	{
		char **copy;

    	copy = (char**)malloc(cli->mCommands.size() * sizeof(char *));
    	for (uint i = 0; i < cli->mCommands.size(); i++) 
		{
			if (!strncmp(cli->mCommands[i].command.c_str(), token, strlen (token))) 
			{
			    copy[total] = strdup(cli->mCommands[i].command.c_str());
			    total++;
			}
    	}
    	*av = copy;
	}
	else
	{
		std::string tmpLine(rl_line_buffer);
		size_t i;
		try
		{
			i = FindCommand(tmpLine); // FindCommands leaves only args in line.
		}
		catch(const std::runtime_error& e)
		{
			return 0;
		}
		if(cli->mCommands[i].address == msgAddr_t::GS_Param_e)
		{
			std::vector<std::string> vec;
			int state = 0;
			bool cont = true;

			while (cont) 
			{
				std::string entry = path_generator(token, state, true);
				if(!entry.empty())
				{
					vec.push_back(entry);
					state = 1;
				}
				else
				{
					cont = false;
				}
				
			}
			if (vec.size() == 0) 
			{
				return 0;
			}
			
			char **array;
			array = (char**)malloc(vec.size() * sizeof(char *));
			if (!array)
			{
				return 0;
			}
			uint i = 0;
			for(auto item : vec)
			{
				array[i++] = dupstr(item.c_str());
			}
			*av = array;
			total = vec.size();
		}
		else
		{
			rl_set_list_possib_func(NULL);
			total = rl_list_possib(token, av);
			rl_set_list_possib_func(&my_rl_list_possib);
		}
		
	}
	
    


    return total;
}

/* Generator function for path completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. rl will free the returned char *, therefore
   we have to  malloc the c_str.*/
std::string CLI::path_generator (const char *text, int state, bool removeBase)
{
	static unsigned int list_index = 0;
	static std::vector<std::string> vec;

	//std::cout << "CLI::path_generator entry: " << list_index << "state: " << state << std::endl;
	/* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
	std::string text_s(text);
	std::string baseText = cli->mFindParamToFind;
	if (!state)
	{
		list_index = 0;
		vec.clear();
		Msg_FindParam::ptr ptr = std::make_unique<Msg_FindParam>(msgAddr_t::GS_Param_e, text_s, "");
		cli->sendMsg(std::move(ptr));

		// Now wait for message to get back
		std::unique_lock<std::mutex> l(cli->mMutex);

		if(!cli->cvFindParam.wait_for(l, std::chrono::milliseconds(5000), [](){return cli->newFindParam; }))
		{
			//throw std::runtime_error("Did not get a find param result");
			return NULL;
		}
		cli->newFindParam = false;

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

	size_t pos = 0;
	if(removeBase)
	{
    	pos = text_s.find_last_of("/");
		if(pos == std::string::npos)
		{
			pos = 0;
		}
		else
		{
			pos++;
		}
	}

	/* Return the next name which partially matches from the command list. */
	if(!vec.empty())
	{
		while(list_index < vec.size())
		{
			list_index++;
			if(0 == vec[list_index-1].find(text_s))
			{
				std::string rtn;
				if(removeBase)
				{
					rtn = vec[list_index-1].substr(pos);
				}
				else
				{
					rtn = vec[list_index-1];
				}
				
				return rtn;
			}
		}
	}
	
	/* If no names matched, then return an empty string. */
	return "";
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
	std::lock_guard<std::mutex> lock(mMutex);

	LOG_DEBUG(log, "Received UI command: " + message->getCommand() + " from " + msgAddrStr.at(message->getSource()));
	mCommands.push_back(UiCommand(message->getCommand(), message->getDoc(), message->getSource()));
}
void CLI::process(Msg_UiCommandResult* message)
{
	std::lock_guard<std::mutex> lock(mMutex);

	newUiRsp = true;
	uiRsp = message->getResult();

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

void CLI::process(Msg_Display* message)
{
	Display(message->getMessage());
}

void CLI::waitForUiResult()
{
	std::unique_lock<std::mutex> l(mMutex);
	if(!cvNewUiRsp.wait_for(l, std::chrono::milliseconds(15), [this](){return newUiRsp; }))
	{
		throw std::runtime_error("Did not get a ui command result");
	}
	newUiRsp = false;
}

void CLI::process(Msg_FlightMode* message)
{
	std::string flightMode;
	try
	{
		flightMode = mapFlightModeToString.at(static_cast<FlightMode_t>(message->getMode()));
	}
	catch(const std::out_of_range & e)
	{
		LOG_ERROR(log, "Received unknown flight mode.");
		return;
	}
	mPromptStatus = flightMode;
	BuildPrompt();
}
void CLI::process(Msg_CtrlMode* message)
{
	std::string controlMode;
	try
	{
		controlMode = mapCtrlModeToString.at(static_cast<CtrlMode_t>(message->getMode()));
	}
	catch(const std::out_of_range & e)
	{
		LOG_ERROR(log, "Received unknown control mode.");
		return;
	}
	mPromptCtrlMode = controlMode;
	BuildPrompt();
}


} /* namespace QuadGS */

