/*
 * main.cpp
 *
 *  Created on: Jan 17, 2015
 *      Author: martin
 */


// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

/* Shows how to use both command line and config file. */

#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
namespace po = boost::program_options;

#include "SerialManager.h"
#include "QuadCLI.h"
#include "QspPayloadRaw.h"
#include "SlipPacket.h"
#include "QuadGSTree.h"
#include "QuadGSTreeValue.h"


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
  logLevel()
  {}
    string logLevel;
    severity_level lvl;
};
}

void handle_program_args(int ac, char* av[], QuadGS::program_args& args);

void msgHandler(QuadGS::QspPayloadRaw::Ptr ptr)
{
  cout << std::endl << "message received in main: " << std::endl << *ptr << std::endl << std::flush;
}

int main(int ac, char* av[])
{

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
    
    QuadGS::Log::Init("app_log", "msg_log", std::clog, args.lvl);

    QuadGS::IoBase::ptr mIO = QuadGS::Serial_Manager::create();
    QuadGS::QuadCLI::ptr mUI = QuadGS::QuadCLI::create();
    QuadGS::Core::ptr mCore = QuadGS::Core::create();

    mUI->registerCommands(mIO->getCommands());
    mUI->registerCommands(mCore->getCommands());
    mUI->SetCore(mCore->getThis());
    mIO->set_read_callback(std::bind(msgHandler, std::placeholders::_1));
    
    
    // Read input.
    while(mUI->ExecuteNextCommand());
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


