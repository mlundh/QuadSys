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

#include "SerialManager.h"
#include "CLI.h"
#ifdef WITH_VISION
	#include "ZedTracker.h"
#endif
#include "QGS_UiInterface.h"
#include "QGS_IoInterface.h"
#include "QGS_CoreInterface.h"
#include "QGS_TrackerInterface.h"


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
};
}

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

    QuadGS::AppLog::Init("app_log", "msg_log", std::clog, args.lvl);
    // Create the modules.
    QuadGS::QGS_IoInterface* mIO = new QuadGS::Serial_Manager();
    QuadGS::QGS_UiInterface* mUI = new QuadGS::CLI();
    QuadGS::QGS_CoreInterface* mCore = new QuadGS::QGS_CoreInterface();
#ifdef WITH_VISION
    QuadGS::QGS_TrackerInterface* mTracker = new QuadGS::ZedTracker();
#endif
    mCore->bind(mIO);
    mCore->bind(mUI);
#ifdef WITH_VISION
    mCore->bind(mTracker);
#endif
    mCore->initialize();
    // Read input.
    while(mUI->RunUI());

    delete mIO;
    delete mUI;
    delete mCore;
#ifdef WITH_VISION
    delete mTracker;
#endif

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
                    ("gui,g", "Gui mode")
                    ("LogLevel,l", po::value< std::string >(&args.logLevel)->default_value("error"),
                            "Log level for outputing to screen.");
    po::variables_map opts;
    po::store(po::parse_command_line(ac, av, generic), opts);
    po::notify(opts);
    if (opts.count("help")) {
        cout << generic << std::endl;
        return;
    }
    if (opts.count("version")) {
        cout << "QuadGS, unofficial version\n";
        return;
    }
    if (opts.count("gui")) {
        cout << "gui mode not yet supported\n";
        //TODO add support for gui mode.
        return;
    }
    if (opts.count("LogLevel")) {
      if(!args.logLevel.compare("error"))
      {
        args.lvl = QuadGS::severity_level::error;
      }
      else if(!args.logLevel.compare("warning"))
      {
        args.lvl = QuadGS::severity_level::warning;
      }
      else if(!args.logLevel.compare("info"))
      {
        args.lvl = QuadGS::severity_level::info;
      }
      else if(!args.logLevel.compare("debug"))
      {
        args.lvl = QuadGS::severity_level::debug;
      }
      else
      {
        cout <<  "Invalid log level, using default: error." << std::endl;
      }
        return;
    }
}


