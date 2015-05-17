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

void Serial_Manager::startRead()
{
  mPort->do_read();
}

void Serial_Manager::write( QspPayloadRaw::Ptr ptr)
{
  mPort->do_write( ptr );
}

 bool Serial_Manager::set_read_callback( )
 {
   return mPort->set_read_callback();
 }


std::vector<Command::ptr> Serial_Manager::getCommands( )
{
  return mCommands;
}


std::string Serial_Manager::getStatus()
{
  std::string s("N/A");
  return s;
}



Serial_Manager::~Serial_Manager() 
{
    mWork.reset();
    mPort.reset();
    mIo_service.stop();
    mThread_io->join();
}

void Serial_Manager::RunThread()
{
    mIo_service.run();
}


} /* namespace QuadGS */
