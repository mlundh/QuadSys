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
#include "IoBase.h"
#include <boost/asio.hpp>
#include <thread>

using namespace boost::asio;
namespace QuadGS {

class Serial_Manager : public IoBase
{
public:
	Serial_Manager();

  virtual ~Serial_Manager();
  
  void write( QspPayloadRaw::Ptr ptr);
  
  virtual void startRead( void );
  
  virtual bool set_read_callback( );
  
  virtual std::vector<Command::ptr> getCommands( );

  virtual std::string getStatus();


private:
    void RunThread();
    
	QuadGS::Serial_Port::pointer mPort;
	boost::asio::io_service mIo_service;
	std::unique_ptr<boost::asio::io_service::work> mWork;
	std::thread *mThread_io;
  std::vector<Command::ptr> mCommands;
};

} /* namespace QuadGS */
#endif /* SERIALMANAGER_H_ */
