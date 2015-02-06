/*
 * Serial_Port.cpp
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */

#include "SerialPort.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <string>
namespace QuadGS {

Serial_Port::Serial_Port(  boost::asio::io_service &io_service ) :
          mName( "unnamed" )
        , mBaudRate( 57600 )
        , mFlowControl( b_a_sp::flow_control::none )
        , mParity( b_a_sp::parity::none )
        , mStopBits( b_a_sp::stop_bits::one )
        , mIoService( io_service )
        , mSerialPort( io_service )
        , mTimeoutRead( io_service )
        , mTimeoutWrite( io_service )
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

void Serial_Port::open( std::string port_name )
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
            //BOOST_LOG_SEV(slg, severity_level::normal) << "error opening port" << ec.message();
        }
        mSerialPort.set_option( b_a_sp::baud_rate( mBaudRate ), ec );
        mSerialPort.set_option( b_a_sp::flow_control( mFlowControl ), ec );
        mSerialPort.set_option( b_a_sp::parity( mParity ), ec );
        mSerialPort.set_option( b_a_sp::stop_bits( mStopBits ), ec );
        if( ec )
        {

            QuadLog(severity_level::error, "Error detected: " + ec.message());
            return;
        }
        QuadLog(severity_level::info, "Serial Port " + port_name + " opened.");
    }
    else
    {
        QuadLog(severity_level::warning, "Serial Port already open");
    }
}

/*Close the serial port*/
void Serial_Port::do_close( const boost::system::error_code& error )
{
    //BOOST_LOG_SEV(slg, severity_level::normal) <<  "..Do_close called";
    if( error )
    {
        QuadLog(severity_level::error,  ".. ..Error detected: " + error.message() );
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
/*Start an async write operation.*/
void Serial_Port::do_write( unsigned char *buffer )
{
    QuadLog(severity_level::info, "Write called.");

    /* First set the timers, otherwise the async_read callback might be called
     * before the timer is started. The timer will then be started afterwards and
     * thus not being canceled by the async_read callback.*/
    mTimeoutWrite.expires_from_now( boost::posix_time::milliseconds( 10000 ) );
    mTimeoutWrite.async_wait(
            boost::bind( & Serial_Port::timer_write_callback,
                    shared_from_this(),
                    boost::asio::placeholders::error ) );

    boost::asio::async_write( mSerialPort, boost::asio::buffer( buffer, 6 ),
            boost::asio::transfer_at_least( 6 ),
            boost::bind( & Serial_Port::write_callback, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );
}

/*Start an async read operation.*/
void Serial_Port::do_read()
{
    QuadLog(severity_level::info, "Read called.");

    /* First set the timers, otherwise the async_read callback might be called
     * before the timer is started. The timer will then be started afterwards and
     * thus not being canceled by the async_read callback.*/
    mTimeoutRead.expires_from_now( boost::posix_time::milliseconds( 10000 ) );
    mTimeoutRead.async_wait(
            boost::bind( & Serial_Port::timer_read_callback,
                    shared_from_this(),
                    boost::asio::placeholders::error ) );

    boost::asio::async_read( mSerialPort,
            boost::asio::buffer( mBufferRead, 6 ),
            boost::asio::transfer_at_least( 6 ),
            boost::bind( & Serial_Port::read_callback, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );

}

/*Callback from write operation.*/
void Serial_Port::write_callback( const boost::system::error_code& error,
        std::size_t bytes_transferred )
{
    QuadLog(severity_level::info, "Write_callback called.");
    if( error )
    {
        QuadLog(severity_level::error, "Read_callback called with an error: " + error.message() );
        do_close( error );
        return;
    }
    QuadLog(severity_level::info, "Bytes transferred: " + std::to_string(bytes_transferred) +  " bytes.");
    mTimeoutWrite.cancel();
}

/*Callback from read operation.*/
void Serial_Port::read_callback( const boost::system::error_code& error,
        std::size_t bytes_transferred )
{
    QuadLog(severity_level::info, "Read_callback called.");
    if( error )
    {
        //BOOST_LOG_SEV(slg, severity_level::error) <<  ".. ..Read_callback called with an error: "
        //          << error.message();
        do_close( error );
        return;
    }

    QuadLog(severity_level::info, "Bytes transferred: " + std::to_string(bytes_transferred) + " bytes.");
    mTimeoutRead.cancel();
}

/* Called when the write timer's deadline expire */
void Serial_Port::timer_write_callback( const boost::system::error_code& error )
{
    //BOOST_LOG_SEV(slg, severity_level::normal) <<  "..timer_write_callback called.";
    if( error )
    {
        //BOOST_LOG_SEV(slg, severity_level::normal) <<  ".. ..Timeout cancelled.";

        /*If the timer was called with the operation_aborted error code then
         * the timer was canceled and did not fire. Everything else is an error*/
        if( error != boost::asio::error::operation_aborted )
            do_close( error );
        return;
    }

    //BOOST_LOG_SEV(slg, severity_level::normal) << ".. ..Timeout fired.";
    mSerialPort.cancel(); // Close all asynchronous operation with serial port. will cause read_callback to fire with an error
    //BOOST_LOG_SEV(slg, severity_level::normal) <<  ".. ..All operations on serial port cancelled.\n";
    return;
}

/* Called when the read timer's deadline expire */
void Serial_Port::timer_read_callback( const boost::system::error_code& error )
{
    //BOOST_LOG_SEV(slg, severity_level::normal) <<  "..timer_read_callback called.";
    if( error )
    {
        //BOOST_LOG_SEV(slg, severity_level::normal) <<  ".. ..Timeout cancelled.";

        /*If the timer was called with the operation_aborted error code then
         * the timer was canceled and did not fire. Everything else is an error*/
        if( error != boost::asio::error::operation_aborted )
        {
            do_close( error );
        }
        return;
    }

    //BOOST_LOG_SEV(slg, severity_level::normal) <<  ".. ..Timeout fired.";
    mSerialPort.cancel(); // Close all asynchronous operation with serial port. will cause read_callback to fire with an error
    //BOOST_LOG_SEV(slg, severity_level::normal) << ".. ..All operations on serial port cancelled.";
    return;
}

Serial_Port::~Serial_Port()
{
    //BOOST_LOG_SEV(slg, severity_level::normal) <<  "Destructor of serial port: ";
    boost::system::error_code error;
    mSerialPort.cancel( error );
    mSerialPort.close( error );
    if( error )
    {
        //BOOST_LOG_SEV(slg, severity_level::error) <<  ".. ..error: " << error.message();
        return;
    }
    //BOOST_LOG_SEV(slg, severity_level::normal) << "  .. finished without error: " ;
}

} /* namespace QuadGS */

