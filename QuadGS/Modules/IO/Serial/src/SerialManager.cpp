/*
 * SerialManager.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */

#include "SerialManager.h"
#include "SlipPacket.h"

namespace QuadGS {
Serial_Manager::Serial_Manager():
        mIo_service()
       ,mWork(new boost::asio::io_service::work(mIo_service))
{
}

void Serial_Manager::Start()
{
  mThread_io = new std::thread(std::bind(&Serial_Manager::RunThread, this));
  mPort = QuadGS::Serial_Port::create(mIo_service);
}


Serial_Manager::ptr Serial_Manager::create()
{
    ptr tmp = ptr(new Serial_Manager);
    tmp->Start();
    return tmp;
}

void Serial_Manager::startRead()
{
  mPort->do_read();
}

void Serial_Manager::write( QspPayloadRaw::Ptr ptr)
{
  mPort->do_write( ptr );
}

void Serial_Manager::set_read_callback(  IoBase::MessageHandlerFcn fcn )
{
  mPort->set_read_callback( fcn );
}

std::string Serial_Manager::openCmd(std::string path)
{
    mPort->open( path );
    return "";
}

std::string Serial_Manager::setNameCmd( std::string port_name )
{
  mPort->set_name(port_name);
  return "";
}

std::string Serial_Manager::setBaudRateCmd( std::string baud )
{
  int baudInt = std::stoi(baud);
  mPort->set_baud_rate(baudInt);
  return "";
}

std::string Serial_Manager::setFlowControlCmd(  std::string flow_ctrl )
{
  int flowCtrl = std::stoi(flow_ctrl);
  mPort->set_flow_control(static_cast<b_a_sp::flow_control::type>(flowCtrl));
  return "";
}

std::string Serial_Manager::setParityCmd( std::string parity )
{
  int par = std::stoi(parity);
  mPort->set_parity(static_cast<b_a_sp::parity::type>(par));
  return "";
}

std::string Serial_Manager::setStopBitsCmd( std::string stop_bits )
{
  int stopBits = std::stoi(stop_bits);
  mPort->set_stop_bits(static_cast<b_a_sp::stop_bits::type>(stopBits));
  return "";
}

std::string Serial_Manager::writeCmd(std::string data)
{
  return "";
}

std::string Serial_Manager::writeRawCmd(std::string data)
{
  QspPayloadRaw::Ptr tmpRaw = QspPayloadRaw::Create(reinterpret_cast<const uint8_t*>(data.c_str()), data.length() );
  mPort->do_write( tmpRaw );
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
  Commands.push_back(std::make_shared<Command> ("writePort",
          std::bind(&Serial_Manager::writeCmd, shared_from_this(), std::placeholders::_1),
          "Write to the serial port.", Command::ActOn::IO));
  Commands.push_back(std::make_shared<Command> ("writePortRawData",
          std::bind(&Serial_Manager::writeRawCmd, shared_from_this(), std::placeholders::_1),
          "Write raw data to the serial port. Data will be interpereted as ascii.", Command::ActOn::IO));
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


} /* namespace QuadGS */
