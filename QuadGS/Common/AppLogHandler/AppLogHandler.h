/*
* AppLogHandler.h
*
* Copyright (C) 2021  Martin Lundh
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
#ifndef __APPLOGHANDLER_H__
#define __APPLOGHANDLER_H__

#include "QGS_Module.h"
#include "AppLog.h"
#include "msgAddr.h"
#include "Msg_GsAppLog.h"
#include "Msg_GetUiCommands.h"
#include "Msg_FireUiCommand.h"
#include <unordered_set>
#include <fstream>

namespace QuadGS {

class AppLogHandler
		: public QGS_ReactiveModule
		, public QGS_MessageHandler<Msg_GsAppLog>
		, public QGS_MessageHandler<Msg_GetUiCommands>
		, public QGS_MessageHandler<Msg_FireUiCommand>

{
public:

	AppLogHandler(msgAddr_t name);

	virtual ~AppLogHandler();


    static QuadGS::log_level logLevelFromStr(std::string level);

    std::string setLogLevelFromStr(std::string level);

    std::string getLogLevel(std::string level);

    std::string listLogLevels();

    std::string listAddresses();

    void process(Msg_GsAppLog* message);

    void process(Msg_GsHasLog* message);

    void process(Msg_GsLogLevel* message);

    void process(Msg_GetUiCommands* message);

    void process(Msg_FireUiCommand* message);

    std::vector<UiCommand> mCommands;
    std::ofstream mAppLogFile;
    std::unordered_set<msgAddr_t> modulesWithLog;
};

}/* namespace QuadGS */
#endif // __APPLOGHANDLER_H__
