/*
 * Serial_Port.cpp
 *
 * Copyright (C) 2015 Martin Lundh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "SerialPort.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <boost/algorithm/string.hpp>

#include "QGS_IoHeader.h"
#include "SlipPacket.h"

namespace QuadGS {

SerialPort::SerialPort(  boost::asio::io_service &io_service ) :
                  AppLog("Serial_Port")
, mName( "unnamed" )
, mBaudRate( 57600 )
, mFlowControl( b_a_sp::flow_control::none )
, mParity( b_a_sp::parity::none )
, mStopBits( b_a_sp::stop_bits::one )
, mIoService( io_service )
, mSerialPort( io_service )
, mTimeoutRead( io_service )
, mTimeoutWrite( io_service )
, mParser()
, mReadTimeoutHandler()
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

void SerialPort::setParser(  ParserBase::ptr parser  )
{
    mParser = parser;

}


void SerialPort::setReadCallback( msgCallbackFcn fcn  )
{
	mMessageHandler = fcn;
    return;
}

void SerialPort::setReadTimeoutCallback(  timeoutHandlerFcn fcn  )
{
    mReadTimeoutHandler = fcn;
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
void SerialPort::write(QGS_ModuleMsgBase::ptr msg)
{
    if( ! mSerialPort.is_open() )
    {
        throw std::runtime_error("Port is not open.");
    }
    if(mWriteBuff.use_count() != 0)
    {
        //error
        QuadLog(severity_level::error, "Write called during ongoing write operation." );
        throw std::runtime_error("Write called during ongoing write operation.");
    }
    //Package into a slip packet and send the packeged data.
    BinaryOStream os;
    if(msg)
    {
    	os << *msg;
    }

    {
		SlipPacket::ptr tmpSlip = SlipPacket::Create(os.get_internal_vec(), true);
		mWriteBuff =  std::make_shared<std::vector<unsigned char> >(std::move(tmpSlip->GetPacket())); // Do not use tmpSlip after move!
    }
    startReadTimer();
    mTimeoutWrite.expires_from_now( boost::posix_time::milliseconds( 10000 ) );
    mTimeoutWrite.async_wait(
            boost::bind( & SerialPort::timerWriteCallback,
                    shared_from_this(),
                    boost::asio::placeholders::error ) );

    boost::asio::async_write( mSerialPort, boost::asio::buffer( *mWriteBuff ),
            transferUntil(SlipPacket::SlipControlOctets::frame_boundary_octet, *mWriteBuff),
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
void SerialPort::read()
{
    if( ! mSerialPort.is_open() )
    {
        return;
    }
    if(mReadBuff.use_count() != 0)
    {
        //error
        QuadLog(severity_level::error, "Read called during read operation." );
        throw std::runtime_error("Read called during read operation.");
    }
    QuadLog(severity_level::debug, "Read called.");
    mReadBuff = std::make_shared<std::vector<unsigned char> >(255,0);;

    boost::asio::async_read( mSerialPort,
            boost::asio::buffer( *mReadBuff),
            transferUntil(SlipPacket::SlipControlOctets::frame_boundary_octet, *mReadBuff),
            boost::bind( & SerialPort::readCallback, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred ) );

}

/*Callback from write operation.*/
void SerialPort::writeCallback( const boost::system::error_code& error,
        std::size_t bytes_transferred )
{
    mWriteBuff.reset();
    try
    {
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


        if( mParser )
        {
        	mReadBuff->erase(mReadBuff->begin()+bytes_transferred, mReadBuff->end()); // remove unused bytes.
            SlipPacket::ptr tmpSlip = SlipPacket::Create(*mReadBuff, false);
            if(mParser->parse( std::make_shared<std::vector<unsigned char> >(std::move(tmpSlip->GetPayload())) ) == 0) // do not use tmpSlip after move!
            {
                if(mMessageHandler)
                {

                	mMessageHandler(mParser->getHeader(), mParser->getPayload());
                }
            }
            else
            {
                QuadLog(severity_level::error,  "Parser error");
            }

        }
        mReadBuff.reset();
        mTimeoutRead.cancel();


    }
    catch (const std::runtime_error& e)
    {
        mReadBuff.reset(); // TODO move this! might cause error if slip creation fails.
        mTimeoutRead.cancel();
        QuadLog(severity_level::error, e.what() );
    }
    read();
}

/* Called when the write timer's deadline expire */
void SerialPort::timerWriteCallback( const boost::system::error_code& error )
{
    mWriteBuff.reset();
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

/* Called when the read timer's deadline expire. Do not cancel payload read, it is still used. */
void SerialPort::timerReadCallback( const boost::system::error_code& error )
{ 
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
    else
    {
        QuadLog(severity_level::warning, " Read timeout fired.");
    }
    if( mReadTimeoutHandler )
    {
        mReadTimeoutHandler();
    }

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

