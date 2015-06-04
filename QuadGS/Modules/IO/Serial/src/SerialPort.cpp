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

Serial_Port::Serial_Port(  boost::asio::io_service &io_service ) :
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


Serial_Port::pointer Serial_Port::create( boost::asio::io_service &io_service )
{
    return Serial_Port::pointer( new Serial_Port( io_service ) );
}

void Serial_Port::close()
{
    mIoService.post(
            boost::bind( & Serial_Port::do_close, this,
                    boost::system::error_code() ) );
}

void Serial_Port::open( std::string port_name )
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

void Serial_Port::set_name( std::string port_name )
{
  mName = port_name;
}

void Serial_Port::set_baud_rate( unsigned int baud_rate )
{
  mBaudRate = baud_rate;
}

void Serial_Port::set_flow_control(  b_a_sp::flow_control::type flow_control )
{
  mFlowControl = flow_control;
}

void Serial_Port::set_parity( b_a_sp::parity::type parity )
{
  mParity = parity;
}

void Serial_Port::set_stop_bits( b_a_sp::stop_bits::type stop_bits )
{
  mStopBits = stop_bits;
}


void Serial_Port::set_read_callback( IoBase::MessageHandlerFcn fcn  )
{
  mMessageHandler = fcn;
    return;
}

/*Close the serial port*/
void Serial_Port::do_close( const boost::system::error_code& error )
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
void Serial_Port::do_write( QspPayloadRaw::Ptr ptr)
{
  if( ! mSerialPort.is_open() )
  {
    return;
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
          boost::bind( & Serial_Port::timer_write_callback,
                  shared_from_this(),
                  boost::asio::placeholders::error ) );

  boost::asio::async_write( mSerialPort, boost::asio::buffer( mPayloadWrite->getPayload(), mPayloadWrite->getPayloadLength() ),
          transfer_until(SlipPacket::SlipControlOctets::frame_boundary_octet, mPayloadWrite->getPayload()),
          boost::bind( & Serial_Port::write_callback, shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred ) );
}


/*Start an async write operation.*/
void Serial_Port::do_write( unsigned char *buffer , uint8_t buffer_length)
{
  if( ! mSerialPort.is_open() )
  {
    throw std::runtime_error("Port is not open.");
  }
  if(mPayloadWrite.use_count() != 0)
  {
      //error
      QuadLog(severity_level::error, "Write called during read operation." );
      throw std::runtime_error("Write called during read operation.");
  }
    QuadLog(severity_level::debug, "Write called.");
    QspPayloadRaw::Ptr tmpRaw = QspPayloadRaw::Create(buffer, buffer_length );

    //Package into a slip packet and send the packeged data.
    SlipPacket::SlipPacketPtr tmpSlip = SlipPacket::Create(tmpRaw, true);
    mPayloadWrite = tmpSlip->GetPacket();
  
    mTimeoutWrite.expires_from_now( boost::posix_time::milliseconds( 10000 ) );
    mTimeoutWrite.async_wait(
            boost::bind( & Serial_Port::timer_write_callback,
                    shared_from_this(),
                    boost::asio::placeholders::error ) );

    boost::asio::async_write( mSerialPort, boost::asio::buffer( mPayloadWrite->getPayload(), mPayloadWrite->getPayloadLength() ),
            transfer_until(SlipPacket::SlipControlOctets::frame_boundary_octet, mPayloadWrite->getPayload()),
            boost::bind( & Serial_Port::write_callback, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );
  }


void Serial_Port::start_read_timer(int timeout)
{
  mTimeoutRead.expires_from_now( boost::posix_time::milliseconds( timeout ) );
  mTimeoutRead.async_wait(
          boost::bind( & Serial_Port::timer_read_callback,
                  shared_from_this(),
                  boost::asio::placeholders::error ) );
}

/*Start an async read operation.*/
void Serial_Port::do_read()
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
            transfer_until(SlipPacket::SlipControlOctets::frame_boundary_octet, mPayloadRead->getPayload()),
            boost::bind( & Serial_Port::read_callback, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );

}

/*Callback from write operation.*/
void Serial_Port::write_callback( const boost::system::error_code& error,
        std::size_t bytes_transferred )
{
  try
  {
    mPayloadWrite.reset();
    if( error )
    {
        QuadLog(severity_level::error, "Read_callback called with an error: " + error.message() );
        do_close( error );
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
void Serial_Port::read_callback( const boost::system::error_code& error,
        std::size_t bytes_transferred )
{
  try
  {
    if( error )
    {
        QuadLog(severity_level::error,  " Read_callback called with an error: " + error.message());
        do_close( error );
    }
    QuadLog(severity_level::debug, "Bytes read: " + std::to_string(bytes_transferred) +  " bytes.");
    
    QspPayloadRaw::Ptr ptr = mPayloadRead;
    ptr->setPayloadLength(bytes_transferred);

    SlipPacket::SlipPacketPtr tmpSlip = SlipPacket::Create(ptr, false);
    
    
    mPayloadRead.reset();
    if( mMessageHandler )
    {
        mMessageHandler( tmpSlip->GetPayload() );
    }
    mTimeoutRead.cancel();

    do_read();
  }
  catch (const std::runtime_error& e)
  {
    QuadLog(severity_level::error, e.what() );
  }
}

/* Called when the write timer's deadline expire */
void Serial_Port::timer_write_callback( const boost::system::error_code& error )
{
    mPayloadWrite.reset();
    if( error )
    {
        /*If the timer was called with the operation_aborted error code then
         * the timer was canceled and did not fire. Everything else is an error*/
        if( error != boost::asio::error::operation_aborted )
        {
            QuadLog(severity_level::error, "Write timer callback: " + error.message());
            do_close( error );
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
void Serial_Port::timer_read_callback( const boost::system::error_code& error )
{ 
  mPayloadRead.reset(); 
  if( error )
    {
        /*If the timer was called with the operation_aborted error code then
         * the timer was canceled and did not fire. Everything else is an error*/
        if( error != boost::asio::error::operation_aborted )
        {
          QuadLog(severity_level::error, "Read timer callback: " + error.message());
          do_close( error );
          throw std::runtime_error(error.message());
        }
        return;
    }

    QuadLog(severity_level::warning, " Read timeout fired.");
    // resend? 
    return;
}

Serial_Port::~Serial_Port()
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

