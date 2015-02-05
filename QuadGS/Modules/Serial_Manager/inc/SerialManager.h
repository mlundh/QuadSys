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
#include <boost/thread.hpp>
#include <boost/bind.hpp>


using namespace boost::asio;
namespace QuadGS {

class Serial_Manager {
public:
	Serial_Manager();


	virtual ~Serial_Manager();




};

} /* namespace QuadGS */
#endif /* SERIALMANAGER_H_ */
