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

namespace QuadGS {

char  CLI::WordBreakPath[] = " \t\n\"\\'`@$><=;|&{(/";
char  CLI::WordBreak[] = " \t\n\"\\'`@$><=;|&{(";

CLI::UiCommand::UiCommand(std::string command, std::string args, std::string address):
		command(command), doc(args), address(address)
{

}

CLI::CLI():
        		 AppLog("CLI"),
				 mPromptStatus("N/A"),
				 mPromptBase("QuadGS"),
				 mPrompt(),
				 mContinue(true)
{
	rl_attempted_completion_function = completion;
	read_history(NULL);
}

CLI::~CLI()
{
	write_history(NULL);
	history_truncate_file(NULL, 100);
}

void CLI::registerCommand(std::string command, std::string doc, std::string address)
{

	mCommands.push_back(UiCommand(command, doc, address));
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

	for(size_t i = 0; i < mCommands.size(); i++)
	{
		if(!mCommands[i].first.compare(command))
		{
			return i;
		}
	}
	throw std::runtime_error("No such command.");
}

std::string CLI::ExecuteLine(std::string line)
{

	size_t i = FindCommand(line);
	if(mCommands[i]->mFunc)
	{
		mLogger(info, "Calling command: " + mCommands[i].first);
		// Send and wait for reply...
		//return (mCommands[i]->mFunc(line));
	}
	else
	{
		throw std::invalid_argument("Command not valid.");
	}
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

std::string CLI::Stop(std::string)
{
	mContinue = false;
	return "";
}
bool CLI::RunUI()
{
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
		QuadLog(error, e.what());
		return true;
	}
	catch(const std::runtime_error& e)
	{
		QuadLog(error, e.what());
		return true;
	}

	catch(const std::exception& e)
	{
		QuadLog(error, e.what());
		return true;
	}
	return mContinue;
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

		switch (mCommands[i]->mActOn)
		{
		case QGS_UiCommand::ActOn::Core:
			rl_completer_word_break_characters = WordBreakPath;
			rl_attempted_completion_over = 1;
			//temporarily change branch for the completion function.
			mCore->mParameters->SaveBranch();
			mCore->mParameters->ChangeBranch(tmpLine);
			matches = rl_completion_matches (text, path_generator);
			// Restore the current branch.
			mCore->mParameters->RestoreBranch();
			break;
		case QGS_UiCommand::ActOn::IO:
			rl_attempted_completion_over = 1;
			break;
		case QGS_UiCommand::ActOn::UI:
			rl_attempted_completion_over = 1;
			break;
		case  QGS_UiCommand::ActOn::File:
			rl_completer_word_break_characters = WordBreak;
			break;
		default:
			break;
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
	while(list_index < mCommands.size())
	{
		list_index++;
		if (strncmp (mCommands[list_index - 1]->mName.c_str(), text, len) == 0)
			return (dupstr(mCommands[list_index - 1]->mName.c_str()));
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
	if (!state)
	{
		list_index = 0;
		vec.clear();
		std::string text_s(text);
		mCore->mParameters->FindPartial(text_s, vec);
	}
	std::string text_s(text);
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
	registerCommand(message->getCommand(), message->getDoc(), message->getSource() );
}
void CLI::process(Msg_UiCommandResult* message)
{
	Display(message->getResult());
}

} /* namespace QuadGS */

