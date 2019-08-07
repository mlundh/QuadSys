/*
 * ParametersTest.cpp
 *
 * Copyright (C) 2017 Martin Lundh
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

#include "Parameters.h"
#include "QGS_Router.h"
#include "CLI.h"
#include "LogHandler.h"
#include "SerialManager.h"
#include "dbgModule.h"


using namespace QuadGS;


int main(int ac, char* av[])
{
	QuadGS::AppLog::Init("app_log", "msg_log", std::clog, severity_level::message_trace, false);

	//Instantiate modules.
	Parameters mParameters(msgAddr_t::GS_Param_e);
	CLI mCli(msgAddr_t::GS_GUI_e);
	LogHandler mLogHandler(msgAddr_t::GS_Log_e);
	Serial_Manager serialIo(msgAddr_t::GS_SerialIO_e);
	dbgModule dbgModule(msgAddr_t::GS_Dbg_e);
	//Instantiate the router.
	QGS_Router mRouter(msgAddr_t::GS_Router_e);

	//Bind all modules to the router.
	mRouter.bind(&mParameters);
	mRouter.bind(&mCli);
	mRouter.bind(&mLogHandler);
	mRouter.bind(&serialIo);
	mRouter.bind(&dbgModule);
	//Run the CLI.
	while(mCli.RunUI());

	return 0;
}
