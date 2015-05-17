/*
 * QuadCLI.h
 *
 *  Created on: March, 2015
 *      Author: martin
 */

#ifndef _QUADCLI_H_
#define _QUADCLI_H_
#include <string>
#include <vector>
#include "Core.h"
#include "QuadGSTree.h"
#include "Log.h"
namespace QuadGS {
  
class QuadCLI : public Log
{
public:
  QuadCLI();
  ~QuadCLI();
  
  bool ExecuteNextCommand();

  void registerCommands(std::vector< Command::ptr > commands);

  void registerTree(QuadGSTree::ptr tree);


private:

  void SetPrompt(std::string prompt);

  std::string ExecuteLine(std::string line);

  void InitCLI();

  void BuildPrompt();

  std::string Stop(std::string);

  static char ** completion (const char *text, int start, int );
  
  static char * command_generator (const char *text, int state);

  static char * path_generator (const char *text, int state);

  static char * dupstr (const char * s);



public:
  static std::vector< Command::ptr > mCommands;
  static QuadGSTree::ptr mTree;
  std::string mPromptStatus;
  std::string mPromptBase;
  std::string mPrompt;
  bool mContinue;

};
} /* namespace QuadGS */

#endif /* _QUADCLI_H_ */
