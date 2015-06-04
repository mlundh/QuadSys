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
#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <functional>
#include "SerialPort.h"
#include "IoBase.h"
#include "Core.h"


using namespace boost::asio;
namespace QuadGS {

class Serial_Manager
    : public IoBase
    , public std::enable_shared_from_this<Serial_Manager>
{
public:
  typedef std::shared_ptr<Serial_Manager> ptr;
  
  virtual ~Serial_Manager();
  void Start();
  
  static ptr create();
  void write( QspPayloadRaw::Ptr ptr);
  virtual void startRead( void );
  virtual void set_read_callback( IoBase::MessageHandlerFcn fcn );
  
  virtual std::vector<Command::ptr> getCommands( );

  virtual std::string getStatus();
  std::string openCmd(std::string path);
  std::string setNameCmd( std::string port_name );
  std::string setBaudRateCmd( std::string baud );
  std::string setFlowControlCmd(  std::string flow_ctrl );
  std::string setParityCmd( std::string parity );
  std::string setStopBitsCmd( std::string stop_bits );
  std::string writeCmd(std::string data);
  std::string writeRawCmd(std::string data);
  std::string startReadCmd(std::string);


private:
  Serial_Manager();
  void RunThread();
    
	QuadGS::Serial_Port::pointer mPort;
	boost::asio::io_service mIo_service;
	std::unique_ptr<boost::asio::io_service::work> mWork;
	std::thread *mThread_io;
};

} /* namespace QuadGS */
#endif /* SERIALMANAGER_H_ */
