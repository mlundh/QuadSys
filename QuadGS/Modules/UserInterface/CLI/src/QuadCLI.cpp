/*
 * QuadCLI.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */

#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>
#include "QuadCLI.h"
#include "Core.h"
#include "IoBase.h"
#include "UiBase.h"
#include <boost/algorithm/string.hpp>

namespace QuadGS {

std::vector<std::shared_ptr < Command > > QuadCLI::mCommands;
Core::ptr QuadCLI::mCore;
char QuadCLI::WordBreakPath[] = " \t\n\"\\'`@$><=;|&{(/";
char QuadCLI::WordBreak[] = " \t\n\"\\'`@$><=;|&{(";

QuadCLI::ptr QuadCLI::create()
{
    ptr tmp = ptr(new QuadCLI);
    return tmp;
}

QuadCLI::QuadCLI():
         Log("CLI"),
         mPromptStatus("N/A"),
         mPromptBase("QuadGS"),
         mPrompt(),
         mContinue(true)
{
    rl_attempted_completion_function = completion;    
    read_history(NULL);
    mCommands.push_back( std::make_shared<Command> ("quit",
            std::bind(&QuadCLI::Stop, this, std::placeholders::_1 ),
            "Quit the application.", Command::ActOn::NoAction));
}

QuadCLI::~QuadCLI()
{
  write_history(NULL);
  history_truncate_file(NULL, 100);
}

void QuadCLI::bind(std::shared_ptr<IoBase> IoPtr)
{
    registerCommands(IoPtr->getCommands());
}

void QuadCLI::registerCommands(std::vector< Command::ptr > commands)
{
    for(size_t i = 0; i < commands.size(); i++)
    {
        std::shared_ptr < Command > tmp = commands[i];
        mCommands.push_back(tmp);
    }

}

size_t QuadCLI::FindCommand(std::string& line)
{
  if(line.empty())
  {
      throw std::runtime_error("Empty line.");
  }

  std::istringstream iss(line);
  std::string fcn;
  iss >> fcn;
  line.erase(line.begin(), line.begin() + fcn.size());
  boost::trim(line);

  for(size_t i = 0; i < mCommands.size(); i++)
  {
      if(!mCommands[i]->mName.compare(fcn))
      {
        return i;
      }
  }
  throw std::runtime_error("No such command.");
}

std::string QuadCLI::ExecuteLine(std::string line)
{
 
  size_t i = FindCommand(line);
  if(mCommands[i]->mFunc)
  {
      return (mCommands[i]->mFunc(line));
  }
  else
  {
      throw std::invalid_argument("Command not valid.");
  }
  return "";
}

void QuadCLI::SetCore(Core::ptr ptr)
{
  mCore = ptr;
}

void QuadCLI::BuildPrompt()
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

std::string QuadCLI::Stop(std::string)
{
    mContinue = false;
    return "";
}
bool QuadCLI::RunUI()
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
char ** QuadCLI::completion (const char *text, int start, int)
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
      size_t i = FindCommand(tmpLine);
      switch (mCommands[i]->mActOn)
      {
        case Command::ActOn::Core:
          rl_completer_word_break_characters = WordBreakPath; 
          rl_attempted_completion_over = 1;
          //temporarily change branch for the completion function.
          mCore->SaveBranch(); 
          mCore->ChangeBranch(tmpLine, false); 
          matches = rl_completion_matches (text, path_generator);
          // Restore the current branch.
          mCore->RestoreBranch();
          break;
        case Command::ActOn::IO:
          rl_attempted_completion_over = 1;
          break; 
        case Command::ActOn::UI:
          rl_attempted_completion_over = 1;
          break;
        case  Command::ActOn::File:
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
char * QuadCLI::command_generator (const char *text, int state)
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
char * QuadCLI::path_generator (const char *text, int state)
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
        mCore->FindPartial(text_s, vec);
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


char * QuadCLI::dupstr (const char * s)
{
    char *r;

    r = (char*)malloc((unsigned long int)(strlen (s) + 1));
    strcpy (r, s);
    return (r);
}
} /* namespace QuadGS */

