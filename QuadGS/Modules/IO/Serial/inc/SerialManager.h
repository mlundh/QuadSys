/*
 * SerialManager.h
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */

#ifndef SERIALMANAGER_H_
#define SERIALMANAGER_H_
#include <boost/asio.hpp>
#include <stdio.h>
#include "SerialPort.h"
#include <boost/asio.hpp>
#include <thread>
#include <boost/bind.hpp>


using namespace boost::asio;
namespace QuadGS {

class Serial_Manager {
public:
	Serial_Manager();


	virtual ~Serial_Manager();

private:
    void RunThread();


	QuadGS::Serial_Port::pointer mPort;
	boost::asio::io_service mIo_service;
	std::unique_ptr<boost::asio::io_service::work> mWork;
	std::thread *mThread_io;
};

} /* namespace QuadGS */
#endif /* SERIALMANAGER_H_ */
