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

#include "../../QGS_Core/CommandTree/QuadGSTree.h"
#include "../../QGS_Core/Core.h"
#include "../../QGS_Core/Log/Log.h"
#include "../../QGS_IO/IoBase.h"
#include "../../QGS_UI/UiBase.h"
namespace QuadGS {
  
class QuadCLI : public Log, public UiBase
{
public:
  
  static UiBase* create();
  
  virtual ~QuadCLI();
  
  virtual bool RunUI();

  virtual void bind(IoBase* IoPtr);

  virtual void registerCommands(std::vector< Command::ptr > commands);
  
  virtual void SetCore(Core* ptr);

  virtual void Display(std::string str);
private:

  QuadCLI();

  void SetPrompt(std::string prompt);

  static size_t FindCommand(std::string& line);

  std::string ExecuteLine(std::string line);

  void BuildPrompt();

  std::string Stop(std::string);

  static char ** completion (const char *text, int start, int );
  
  static char * command_generator (const char *text, int state);

  static char * path_generator (const char *text, int state);

  static char * dupstr (const char * s);



public:
  static std::vector< Command::ptr > mCommands;
  static Core* mCore;
  std::string mPromptStatus;
  std::string mPromptBase;
  std::string mPrompt;
  bool mContinue;
  static char WordBreakPath[];
  static char WordBreak[];
  


};
} /* namespace QuadGS */

#endif /* _QUADCLI_H_ */
