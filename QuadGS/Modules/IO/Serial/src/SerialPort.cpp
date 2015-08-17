/*
 * Serial_Port.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */

#include "SerialPort.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "QspPayloadRaw.h"
#include "SlipPacket.h"
#include <boost/algorithm/string.hpp>

namespace QuadGS {

SerialPort::SerialPort(  boost::asio::io_service &io_service ) :
          Log("Serial_Port")
        , mName( "unnamed" )
        , mBaudRate( 57600 )
        , mFlowControl( b_a_sp::flow_control::none )
        , mParity( b_a_sp::parity::none )
        , mStopBits( b_a_sp::stop_bits::one )
        , mIoService( io_service )
        , mSerialPort( io_service )
        , mTimeoutRead( io_service )
        , mTimeoutWrite( io_service )
        , mMessageHandler()
{
    if( mSerialPort.is_open() )
    {
        throw std::runtime_error( "The serial port is already open." );
        return;
    }


}


SerialPort::ptr SerialPort::create( boost::asio::io_service &io_service )
{
    return SerialPort::ptr( new SerialPort( io_service ) );
}

void SerialPort::close()
{
    mIoService.post(
            boost::bind( & SerialPort::doClose, this,
                    boost::system::error_code() ) );
}

void SerialPort::open( std::string port_name )
{
  boost::trim(port_name);
  if(port_name.empty())
  {
    port_name = mName;
  }
  
  boost::system::error_code ec;
  if( ! mSerialPort.is_open() )
  {
      mSerialPort.open( port_name, ec );

      if( ec )
      { 
        throw std::runtime_error(ec.message());
      }
      mSerialPort.set_option( b_a_sp::baud_rate( mBaudRate ), ec );
      mSerialPort.set_option( b_a_sp::flow_control( mFlowControl ), ec );
      mSerialPort.set_option( b_a_sp::parity( mParity ), ec );
      mSerialPort.set_option( b_a_sp::stop_bits( mStopBits ), ec );
      if( ec )
      {

          throw std::runtime_error(ec.message());
      }
      QuadLog(severity_level::debug, "Serial Port " + port_name + " opened.");
  }
  else
  {
      QuadLog(severity_level::warning, "Serial Port already open");
      return;
  }
  return;
}

void SerialPort::setName( std::string port_name )
{
  mName = port_name;
}

void SerialPort::setBaudRate( unsigned int baud_rate )
{
  mBaudRate = baud_rate;
}

void SerialPort::setFlowControl(  b_a_sp::flow_control::type flow_control )
{
  mFlowControl = flow_control;
}

void SerialPort::setParity( b_a_sp::parity::type parity )
{
  mParity = parity;
}

void SerialPort::setStopBits( b_a_sp::stop_bits::type stop_bits )
{
  mStopBits = stop_bits;
}


void SerialPort::setReadCallback( IoBase::MessageHandlerFcn fcn  )
{
  mMessageHandler = fcn;
    return;
}

/*Close the serial port*/
void SerialPort::doClose( const boost::system::error_code& error )
{
  if( error )
    {
        QuadLog(severity_level::error, "Close: "+ error.message() );
        throw std::runtime_error(error.message());
    }
    if( error == boost::asio::error::operation_aborted )
    {
        return;
    }
    mTimeoutRead.cancel();
    mTimeoutWrite.cancel();
    mSerialPort.close();
    QuadLog(severity_level::info, "Serial Port closed");
    return;
}
void SerialPort::doWrite( QspPayloadRaw::Ptr ptr)
{
  if( ! mSerialPort.is_open() )
  {
      throw std::runtime_error("Port is not open.");
  }
  if(mPayloadWrite.use_count() != 0)
  {
      //error
      QuadLog(severity_level::error, "Write called during ongoing write operation." );
      throw std::runtime_error("Write called during ongoing write operation.");
  }
  //Package into a slip packet and send the packeged data.
  SlipPacket::SlipPacketPtr tmpSlip = SlipPacket::Create(ptr, true);
  mPayloadWrite = tmpSlip->GetPacket();
  
  mTimeoutWrite.expires_from_now( boost::posix_time::milliseconds( 10000 ) );
  mTimeoutWrite.async_wait(
          boost::bind( & SerialPort::timerWriteCallback,
                  shared_from_this(),
                  boost::asio::placeholders::error ) );

  boost::asio::async_write( mSerialPort, boost::asio::buffer( mPayloadWrite->getPayload(), mPayloadWrite->getPayloadLength() ),
          transferUntil(SlipPacket::SlipControlOctets::frame_boundary_octet, mPayloadWrite->getPayload()),
          boost::bind( & SerialPort::writeCallback, shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred ) );
}

void SerialPort::startReadTimer(int timeout)
{
  mTimeoutRead.expires_from_now( boost::posix_time::milliseconds( timeout ) );
  mTimeoutRead.async_wait(
          boost::bind( & SerialPort::timerReadCallback,
                  shared_from_this(),
                  boost::asio::placeholders::error ) );
}

/*Start an async read operation.*/
void SerialPort::doRead()
{
    if( ! mSerialPort.is_open() )
    {
        return;
    }
    if(mPayloadRead.use_count() != 0)
    {
        //error
        QuadLog(severity_level::error, "Read called during read operation." );
        throw std::runtime_error("Read called during read operation.");
    }
    QuadLog(severity_level::debug, "Read called.");
    mPayloadRead = QspPayloadRaw::Create(255);

    boost::asio::async_read( mSerialPort,
            boost::asio::buffer( mPayloadRead->getPayload(), mPayloadRead->getPayloadLength() ),
            transferUntil(SlipPacket::SlipControlOctets::frame_boundary_octet, mPayloadRead->getPayload()),
            boost::bind( & SerialPort::readCallback, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );

}

/*Callback from write operation.*/
void SerialPort::writeCallback( const boost::system::error_code& error,
        std::size_t bytes_transferred )
{
  try
  {
    mPayloadWrite.reset();
    if( error )
    {
        QuadLog(severity_level::error, "Read_callback called with an error: " + error.message() );
        doClose( error );
    }
    QuadLog(severity_level::debug, "Bytes written: " + std::to_string(bytes_transferred) +  " bytes.");
    mTimeoutWrite.cancel();
  }
  catch (const std::runtime_error& e)
  {
    QuadLog(severity_level::error, e.what() );
  }
}

/*Callback from read operation.*/
void SerialPort::readCallback( const boost::system::error_code& error,
        std::size_t bytes_transferred )
{
  try
  {
    if( error )
    {
        QuadLog(severity_level::error,  " Read_callback called with an error: " + error.message());
        doClose( error );
    }
    QuadLog(severity_level::debug, "Bytes read: " + std::to_string(bytes_transferred) +  " bytes.");
    
    QspPayloadRaw::Ptr ptr = mPayloadRead;
    ptr->setPayloadLength(static_cast<uint16_t>(bytes_transferred));

    SlipPacket::SlipPacketPtr tmpSlip = SlipPacket::Create(ptr, false);
    
    
    mPayloadRead.reset();
    if( mMessageHandler )
    {
        mMessageHandler( tmpSlip->GetPayload() );
    }
    mTimeoutRead.cancel();


  }
  catch (const std::runtime_error& e)
  {
    mPayloadRead.reset(); // TODO move this! might cause error if slip creation fails.
    mTimeoutRead.cancel();
    QuadLog(severity_level::error, e.what() );
  }
  doRead();
}

/* Called when the write timer's deadline expire */
void SerialPort::timerWriteCallback( const boost::system::error_code& error )
{
    mPayloadWrite.reset();
    if( error )
    {
        /*If the timer was called with the operation_aborted error code then
         * the timer was canceled and did not fire. Everything else is an error*/
        if( error != boost::asio::error::operation_aborted )
        {
            QuadLog(severity_level::error, "Write timer callback: " + error.message());
            doClose( error );
            throw std::runtime_error(error.message());
        }
        return;
    }

    QuadLog(severity_level::warning, "Write timeout fired.");
    mSerialPort.cancel(); // Close all asynchronous operation with serial port. 
                          //will cause read_callback to fire with an error
    return;
}

/* Called when the read timer's deadline expire */
void SerialPort::timerReadCallback( const boost::system::error_code& error )
{ 
  mPayloadRead.reset(); 
  if( error )
    {
        /*If the timer was called with the operation_aborted error code then
         * the timer was canceled and did not fire. Everything else is an error*/
        if( error != boost::asio::error::operation_aborted )
        {
          QuadLog(severity_level::error, "Read timer callback: " + error.message());
          doClose( error );
          throw std::runtime_error(error.message());
        }
        return;
    }

    QuadLog(severity_level::warning, " Read timeout fired.");
    // resend? 
    return;
}

SerialPort::~SerialPort()
{
    boost::system::error_code error;
    mSerialPort.cancel( error );
    mSerialPort.close( error );
    if( error )
    {
        throw std::runtime_error(error.message());
    }
}

} /* namespace QuadGS */

