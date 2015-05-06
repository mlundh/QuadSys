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

#include "SerialPort.h"
#include "QuadCLI.h"
#include "SlipPacket.h"
#include "CommandBase.h"
#include "CommandValue.h"


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
  open(false)
  {}
    std::string port_name;
    bool open;
};
}

void handle_program_args(int ac, char* av[], QuadGS::program_args& args);
//bool execute_next_command(std::istream& input_stream, std::ostream& output_stream);



int main(int ac, char* av[])
{
    QuadGS::Log::Init("app_log", "msg_log");
    QuadGS::program_args args;
    try {
        handle_program_args(ac, av, args);
    }
    catch(exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    QuadGS::CommandBase* testCommand = new QuadGS::CommandBase("Parameter");
    testCommand->Register("Group1<1>/value1");
    testCommand->Register("Group1/value2<3>");
    testCommand->Register("Group1/value3/test1");
    testCommand->Register("Group3");
    testCommand->Register("Group4");


    testCommand->SetValue("Group1/value1", "23");
    std::string value;
    testCommand->GetValue("Group1/value1", value);

    cout << "Group1/value1 has value: " << value << std::endl;
    // Read input.
    QuadGS::QuadCLI::InitCLI();
    while(QuadGS::QuadCLI::ExecuteNextCommand());

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
                    ("port,p", po::value< std::string >(&args.port_name)->default_value("/dev/ttyUSB0"),
                            "Port used for communication.")
                    ("open,o", po::value< bool >(&args.open)->default_value(false));
    po::variables_map opts;
    po::store(po::parse_command_line(ac, av, generic), opts);
    po::notify(opts);
    if (opts.count("help")) {
        cout << generic << "\n";
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
}


