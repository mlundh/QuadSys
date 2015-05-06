/*
 * SerialManager.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */

#include "SerialManager.h"

namespace QuadGS {
Serial_Manager::Serial_Manager():
        mIo_service()
       ,mWork(new boost::asio::io_service::work(mIo_service))
{

    mThread_io = new std::thread(std::bind(&Serial_Manager::RunThread, this));
    mPort = QuadGS::Serial_Port::create(mIo_service);
}

Serial_Manager::~Serial_Manager() {
    mWork.reset();
    mThread_io->join();
}

void Serial_Manager::RunThread()
{
    mIo_service.run();
}


} /* namespace QuadGS */
