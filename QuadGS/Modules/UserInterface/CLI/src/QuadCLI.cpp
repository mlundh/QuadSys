/*
 * QuadCLI.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */
#include "QuadCLI.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace QuadGS {
  
namespace QuadCLI{

char ** completion (const char *text, int start, int end);
char * command_generator (const char *text, int state);
char * dupstr (const char * s);

class Command{
public:
    typedef std::function<bool(std::string)> fcn;
    Command(std::string name, Command::fcn func, std::string doc):
         mName(name)
        ,mFunc(func)
        ,mDoc(doc)
{}
  std::string mName;           /* User printable name of the function. */
  fcn mFunc;                   /* Function to call to do the job. */
  std::string mDoc;            /* Documentation for this function.  */
};

std::vector<Command*> commands;
std::string mPromptStatus;
std::string mPromptBase;
std::string mPrompt;
void InitCLI()
{
    rl_attempted_completion_function = completion;
    rl_completion_entry_function = command_generator;
    mPromptStatus = "NA";
    mPromptBase = "QuadGS";
    commands.push_back( new Command("quit", (Command::fcn)NULL, "Quit the application."));
    commands.push_back( new Command("set", (Command::fcn)NULL, "Set the value in the tree."));
    commands.push_back( new Command("get", (Command::fcn)NULL, "Set the value in the tree."));
}
void BuildPrompt()
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

bool ExecuteNextCommand()
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


        if (line.compare("quit") == 0 || line.compare("q") == 0 )
            return false;

        add_history (line.c_str());
    }
    catch(std::exception& e)
    {
        std::cerr << "Console error: " << e.what() << "\n";
        return true;
    }
    return true;
}

/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
char ** completion (const char *text, int start, int )
{
  char **matches;

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0)
    matches = rl_completion_matches (text, command_generator);
  else
    matches = rl_completion_matches (text, command_generator);
  return (matches);
}

/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. rl will free the returned char *, therefore
   we have to  malloc the c_str.*/
char * command_generator (const char *text, int state)
{
  static unsigned int list_index, len;
  std::string name;

  /* If this is a new word to complete, initialize now.  This includes
     saving the length of TEXT for efficiency, and initializing the index
     variable to 0. */
  if (!state)
    {
      list_index = 0;
      len = strlen (text);
    }

  /* Return the next name which partially matches from the command list. */
  while(list_index < commands.size())
  {
      list_index++;
      if (strncmp (commands[list_index - 1]->mName.c_str(), text, len) == 0)
        return (dupstr(commands[list_index - 1]->mName.c_str()));
  }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}



char * dupstr (const char * s)
{
  char *r;

  r = (char*)malloc((unsigned long int)(strlen (s) + 1));
  strcpy (r, s);
  return (r);
}
}
} /* namespace QuadGS */

