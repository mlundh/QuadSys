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

#include "Serial_Manager/inc/SerialPort.h"
#include <iostream>
#include <fstream>
#include <iterator>
using namespace std;
typedef boost::asio::serial_port b_a_sp;

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
    return os;
}


boost::asio::io_service io_service;
auto_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(io_service));
unsigned char mBufferWrite[6] = {0,1,2,5,4,5};
void run()
{
  io_service.run();
}


int main(int ac, char* av[])
{
    try {
        QuadGS::Log::Init("first", "second");
        // Declare a group of options that will be
        // allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()
            ("help,h", "produce help message");

        po::options_description cmdline_options;
        cmdline_options.add(generic);

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, generic), vm);

        if (vm.count("help")) {
            cout << generic << "\n";
            return 0;
        }

        if (vm.count("version")) {
            cout << "Multiple sources example, version 1.0\n";
            return 0;
        }
    }
    catch(exception& e)
    {
    	cout << e.what() << "\n";
    	return 1;
    }
    try {
    	QuadGS::Serial_Port::pointer port = QuadGS::Serial_Port::create(io_service);
    	boost::thread thread_io(run);


    	port->open("/dev/ttyUSB0");
    	port->do_write(mBufferWrite);
    	port->do_read();
    }
    catch(exception& e)
    {
    	cout << e.what() << "\n";
       // return 1;
    }
    cout << "Press any key to exit..."<<endl;
       cin.get();
    work.reset();
    return 0;
}
