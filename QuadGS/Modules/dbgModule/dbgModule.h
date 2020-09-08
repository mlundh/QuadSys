/*
 * dbgModule.h
 *
 *
 * Copyright (C) 2018 Martin Lundh
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

#ifndef QUADGS_MODULES_DBGMODULE_DBGMODULE_H_
#define QUADGS_MODULES_DBGMODULE_DBGMODULE_H_

#include "QGS_Module.h"
#include "Msg_GetUiCommands.h"
#include "Msg_FireUiCommand.h"
#include "Msg_Display.h"
#include "Msg_Debug.h"

#include "Msg_Test.h"

namespace QuadGS {

class dbgModule
: public QGS_ReactiveModule
, public QGS_MessageHandler<Msg_GetUiCommands>
, public QGS_MessageHandler<Msg_FireUiCommand>
, public QGS_MessageHandler<Msg_Debug>

{

public:



	dbgModule(msgAddr_t name, msgAddr_t dbgAddr);

	virtual ~dbgModule();

	std::string sendUiMsg(std::string msg);

	std::string getRuntimeStats(std::string);

	std::string BindRc(std::string);

	virtual void process(Msg_GetUiCommands* message);
	virtual void process(Msg_FireUiCommand* message);
	virtual void process(Msg_Debug* message);

	std::vector<UiCommand> mCommands;
	msgAddr_t mDbgAddr;
};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_DBGMODULE_DBGMODULE_H_ */
