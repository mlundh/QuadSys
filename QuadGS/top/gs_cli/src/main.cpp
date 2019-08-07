/*
 * main.cpp
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

#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
namespace po = boost::program_options;

#include "Parameters.h"
#include "QGS_Router.h"
#include "CLI.h"
#include "LogHandler.h"
#include "SerialManager.h"
#include "dbgModule.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <vector>
#include <memory>
#include <boost/iterator/transform_iterator.hpp>
using namespace std;
typedef boost::asio::serial_port b_a_sp;

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
    return os;
}
const char *convert_to_cstr(const std::string & s)
{
   return s.c_str();
}

namespace QuadGS
{
struct program_args
{
  program_args():
  logLevel(),
  lvl()
  {}
    string logLevel;
    severity_level lvl;
    bool msgTrace;
};
}
using namespace QuadGS;
void handle_program_args(int ac, char* av[], QuadGS::program_args& args);

int main(int ac, char* av[])
{
    // Handle program options.
    QuadGS::program_args args;
    try
    {
        handle_program_args(ac, av, args);
    }
    catch(exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    //Initialize the app log.
    QuadGS::AppLog::Init("app_log", "msg_log", std::clog, args.lvl, args.msgTrace);

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

    while(mCli.RunUI());

    return 0;
}

//Handles program args. Throws if not successful.
void handle_program_args(int ac, char* av[], QuadGS::program_args& args)
{

    // Parse program options.
    po::options_description generic("Generic options");
    generic.add_options()
                    ("help,h", "produce help message")
                    ("version,v", "print version string")
                    ("LogLevel,l", po::value< std::string >(&args.logLevel)->default_value("error"),
                            "Log level for outputing to screen.")
					("msgTrace,t",po::value< bool >(&args.msgTrace)->default_value(false), "Trace all messages in the system to file.");
    po::variables_map opts;
    po::store(po::parse_command_line(ac, av, generic), opts);
    po::notify(opts);
    if (opts.count("help"))
    {
        cout << generic << std::endl;
        return;
    }
    if (opts.count("version"))
    {
        cout << "QuadGS, unofficial version\n";
        return;
    }
    if (opts.count("LogLevel"))
    {
    	args.lvl = AppLog::logLevelFromStr(args.logLevel);
    	if(args.lvl == severity_level::invalid)
    	{
    		args.lvl = QuadGS::error;
    		cout << "Invalid log level. Using default: error." << std::endl;
    	}
    }
    return;
}


