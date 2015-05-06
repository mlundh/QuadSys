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
        , mErrorHandler()
{
    if( mSerialPort.is_open() )
    {
        throw std::runtime_error( "The serial port is already open." );
        return;
    }


}


/*std::string port_name, unsigned int baud_rate,
        b_a_sp::flow_control::type flow_control, b_a_sp::parity::type parity,
        b_a_sp::stop_bits::type stop_bits,*/


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

bool Serial_Port::open( std::string port_name )
{
    boost::system::error_code ec;
    if( ! mSerialPort.is_open() )
    {
        mSerialPort.open( port_name, ec );

        if( ec == boost::system::errc::no_such_file_or_directory )
        { //for example you tried to open "COM1" on a linux machine.
          //... handle the error somehow
        }
        else if( ec )
        {
          QuadLog(severity_level::error, ec.message());
          return false;
        }
        mSerialPort.set_option( b_a_sp::baud_rate( mBaudRate ), ec );
        mSerialPort.set_option( b_a_sp::flow_control( mFlowControl ), ec );
        mSerialPort.set_option( b_a_sp::parity( mParity ), ec );
        mSerialPort.set_option( b_a_sp::stop_bits( mStopBits ), ec );
        if( ec )
        {

            QuadLog(severity_level::error, ec.message());
            return false;
        }
        QuadLog(severity_level::info, "Serial Port " + port_name + " opened.");
    }
    else
    {
        QuadLog(severity_level::warning, "Serial Port already open");
        return true;
    }
    return true;
}

bool Serial_Port::set_read_callback(  )
{
    if(1)
    {
        //mMessageHandler;
        return true;
    }
    return false;
}

/*Close the serial port*/
void Serial_Port::do_close( const boost::system::error_code& error )
{
  if( error )
    {
        QuadLog(severity_level::error, "Close: "+ error.message() );
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
      return;
  }
    mPayloadWrite = ptr;
    mTimeoutWrite.expires_from_now( boost::posix_time::milliseconds( 10000 ) );
    mTimeoutWrite.async_wait(
            boost::bind( & Serial_Port::timer_write_callback,
                    shared_from_this(),
                    boost::asio::placeholders::error ) );

    boost::asio::async_write( mSerialPort, boost::asio::buffer( mPayloadWrite->getPayload(), mPayloadWrite->getPayloadLength() ),
            transfer_until(0x7E, mPayloadWrite->getPayload()),
            boost::bind( & Serial_Port::write_callback, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );
}


/*Start an async write operation.*/
void Serial_Port::do_write( unsigned char *buffer , uint8_t buffer_length)
{
  if( ! mSerialPort.is_open() )
  {
    return;
  }
    QuadLog(severity_level::debug, "Write called.");
    std::string myString(buffer, buffer + buffer_length);
    QuadLog(severity_level::info, myString);
    /* First set the timers, otherwise the async_read callback might be called
     * before the timer is started. The timer will then be started afterwards and
     * thus not being canceled by the async_read callback.*/
    mTimeoutWrite.expires_from_now( boost::posix_time::milliseconds( 10000 ) );
    mTimeoutWrite.async_wait(
            boost::bind( & Serial_Port::timer_write_callback,
                    shared_from_this(),
                    boost::asio::placeholders::error ) );

    boost::asio::async_write( mSerialPort, boost::asio::buffer( buffer, buffer_length ),
            boost::asio::transfer_at_least( buffer_length ),
            boost::bind( & Serial_Port::write_callback, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );
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
        return;
    }
    QuadLog(severity_level::debug, "Read called.");
    mPayloadRead = QspPayloadRaw::Create(255);
    /* First set the timers, otherwise the async_read callback might be called
     * before the timer is started. The timer will then be started afterwards and
     * thus not being canceled by the async_read callback.*/
    mTimeoutRead.expires_from_now( boost::posix_time::milliseconds( 10000 ) );
    mTimeoutRead.async_wait(
            boost::bind( & Serial_Port::timer_read_callback,
                    shared_from_this(),
                    boost::asio::placeholders::error ) );

    boost::asio::async_read( mSerialPort,
            boost::asio::buffer( mPayloadRead->getPayload(), mPayloadRead->getPayloadLength() ),
            transfer_until(0x7E, mPayloadRead->getPayload()),
            boost::bind( & Serial_Port::read_callback, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );

}

/*Callback from write operation.*/
void Serial_Port::write_callback( const boost::system::error_code& error,
        std::size_t bytes_transferred )
{
    mPayloadWrite.reset();
    if( error )
    {
        QuadLog(severity_level::error, "Read_callback called with an error: " + error.message() );
        do_close( error );
        return;
    }
    QuadLog(severity_level::debug, "Bytes written: " + std::to_string(bytes_transferred) +  " bytes.");
    mTimeoutWrite.cancel();
}

/*Callback from read operation.*/
void Serial_Port::read_callback( const boost::system::error_code& error,
        std::size_t bytes_transferred )
{
    if( error )
    {
        QuadLog(severity_level::error,  " Read_callback called with an error: " + error.message());
        do_close( error );
        return;
    }
    QspPayloadRaw::Ptr ptr = mPayloadRead;
    ptr->setPayloadLength(bytes_transferred);
    mPayloadRead.reset();
    if( mMessageHandler )
    {
        mMessageHandler( );// mPayloadWrite );
    }
    mTimeoutRead.cancel();

    do_read();
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
{  if( error )
    {
        /*If the timer was called with the operation_aborted error code then
         * the timer was canceled and did not fire. Everything else is an error*/
        if( error != boost::asio::error::operation_aborted )
        {
          QuadLog(severity_level::error, "Read timer callback: " + error.message());
          do_close( error );
        }
        return;
    }

    QuadLog(severity_level::warning, " Read timeout fired.");
    mSerialPort.cancel(); // Close all asynchronous operation with serial port. 
                          // will cause read_callback to fire with an error
    return;
}

Serial_Port::~Serial_Port()
{
    QuadLog(severity_level::debug, "In destructor");
  boost::system::error_code error;
    mSerialPort.cancel( error );
    mSerialPort.close( error );
    if( error )
    {
         QuadLog(severity_level::error, "In destructor: " + error.message());
        return;
    }
}

} /* namespace QuadGS */

