/*
 * SerialManager.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */

#include "SerialManager.h"
#include "SlipPacket.h"
#include "QuadSerialPacket.h"
#include <boost/algorithm/string.hpp>

namespace QuadGS {
Serial_Manager::Serial_Manager():
        mIo_service()
       ,mWork(new boost::asio::io_service::work(mIo_service))
       ,mFifo(1, 1000)
       ,mRetries(0)
       ,mLog("SerialManager")
{
}

void Serial_Manager::Start()
{
  mThread_io = new std::thread(std::bind(&Serial_Manager::RunThread, this));
  mPort = QuadGS::SerialPort::create(mIo_service);
  mPort->setReadCallback( std::bind(&Serial_Manager::messageHandler, shared_from_this(), std::placeholders::_1) );
  mPort->setReadTimeoutCallback( std::bind(&Serial_Manager::timeoutHandler, shared_from_this()) );

}


Serial_Manager::ptr Serial_Manager::create()
{
    ptr tmp = ptr(new Serial_Manager);
    tmp->Start();
    return tmp;
}

void Serial_Manager::startRead()
{
  mPort->Read();
}

void Serial_Manager::write( QspPayloadRaw::Ptr ptr)
{
  mFifo.push(ptr);
  doWrite();
}

void Serial_Manager::setReadCallback(  IoBase::MessageHandlerFcn fcn )
{
    mMessageHandler = fcn;
}

std::string Serial_Manager::openCmd(std::string path)
{
    mPort->open( path );
    startRead();
    return "";
}

std::string Serial_Manager::setNameCmd( std::string port_name )
{
  mPort->setName(port_name);
  return "";
}

std::string Serial_Manager::setBaudRateCmd( std::string baud )
{
  int baudInt = std::stoi(baud);
  mPort->setBaudRate(baudInt);
  return "";
}

std::string Serial_Manager::setFlowControlCmd(  std::string flow_ctrl )
{
  int flowCtrl = std::stoi(flow_ctrl);
  mPort->setFlowControl(static_cast<b_a_sp::flow_control::type>(flowCtrl));
  return "";
}

std::string Serial_Manager::setParityCmd( std::string parity )
{
  int par = std::stoi(parity);
  mPort->setParity(static_cast<b_a_sp::parity::type>(par));
  return "";
}

std::string Serial_Manager::setStopBitsCmd( std::string stop_bits )
{
  int stopBits = std::stoi(stop_bits);
  mPort->setStopBits(static_cast<b_a_sp::stop_bits::type>(stopBits));
  return "";
}

std::string Serial_Manager::startReadCmd(std::string)
{
  startRead();
  return "";
}

std::vector<Command::ptr> Serial_Manager::getCommands( )
{
  std::vector<std::shared_ptr < Command > > Commands;
  Commands.push_back(std::make_shared<Command> ("setPort",
          std::bind(&Serial_Manager::setNameCmd, shared_from_this(), std::placeholders::_1),
          "Set the port name.", Command::ActOn::File));
  Commands.push_back(std::make_shared<Command> ("setBaudRate",
          std::bind(&Serial_Manager::setBaudRateCmd, shared_from_this(), std::placeholders::_1),
          "Set the baud rate of the port, default 57600 baud.", Command::ActOn::IO));
  Commands.push_back(std::make_shared<Command> ("setFlowControl",
          std::bind(&Serial_Manager::setFlowControlCmd, shared_from_this(), std::placeholders::_1),
          "Set flow control, default None.", Command::ActOn::IO));
  Commands.push_back(std::make_shared<Command> ("setParity",
          std::bind(&Serial_Manager::setParityCmd, shared_from_this(), std::placeholders::_1),
          "Set parity, default none.", Command::ActOn::IO));
  Commands.push_back(std::make_shared<Command> ("setStopBits",
          std::bind(&Serial_Manager::setStopBitsCmd, shared_from_this(), std::placeholders::_1),
          "Set number of stop bits to be used, default one.", Command::ActOn::IO));
  Commands.push_back(std::make_shared<Command> ("openPort",
          std::bind(&Serial_Manager::openCmd, shared_from_this(), std::placeholders::_1),
          "Open the serial port.", Command::ActOn::File));
  Commands.push_back(std::make_shared<Command> ("startReadPort",
          std::bind(&Serial_Manager::startReadCmd, shared_from_this(), std::placeholders::_1),
          "Start the read operation.", Command::ActOn::IO));

  return Commands;
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
  try
  {
    mIo_service.run();
  }
  catch(...)
  {
    std::cout << "exception from  io service!" << std::endl;
  }
}

void Serial_Manager::timeoutHandler()
{
    if(mRetries < 2)
    {
        mRetries++;
        doWrite();
        mLog.QuadLog(severity_level::warning, "No reply, retrying");

    }
    else
    {
        mRetries = 0;
        mFifo.pop();
        mLog.QuadLog(severity_level::error, "Transmission failed.");
    }
}

void Serial_Manager::messageHandler( QspPayloadRaw::Ptr ptr)
{
    mFifo.pop();
    QuadSerialPacket::Ptr packetPtr = QuadSerialPacket::Create(ptr);
    if( mMessageHandler )
    {
        mMessageHandler( packetPtr );
    }

}
void Serial_Manager::doWrite()
{
    mPort->Write( mFifo.front() );
}




} /* namespace QuadGS */
