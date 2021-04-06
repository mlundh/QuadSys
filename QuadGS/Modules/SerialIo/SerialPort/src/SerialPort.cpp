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

#include "SlipPacket.h"

namespace QuadGS {

SerialPort::SerialPort(  boost::asio::io_service &io_service, sendFunction_t logSendFcn) 
: mName( "unnamed" )
, mBaudRate( 460800 )
, mFlowControl( b_a_sp::flow_control::none )
, mParity( b_a_sp::parity::none )
, mStopBits( b_a_sp::stop_bits::one )
, mIoService( io_service )
, mSerialPort( io_service )
, mTimeoutWrite( io_service )
, mWriteOngoing ( false )
, mParser()
, mReadTimeoutHandler()
, mWriteCallback()
, log("SerialPort",logSendFcn)
, asioLog("SerialPort",logSendFcn)
{
	if( mSerialPort.is_open() )
	{
		throw std::runtime_error( "The serial port is already open." );
		return;
	}

}


SerialPort::ptr SerialPort::create( boost::asio::io_service &io_service, sendFunction_t logSendFcn)
{
	return SerialPort::ptr( new SerialPort( io_service, logSendFcn) );
}

void SerialPort::close()
{
	mIoService.post(
			boost::bind( & SerialPort::doClose, this,
					boost::system::error_code() ) );
}

std::string SerialPort::open( std::string port_name )
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
		LOG_DEBUG(log, "Serial Port " << port_name << " opened.");
	}
	else
	{
		LOG_WARNING(log, "Serial Port already open");
		return "";
	}
	return "Port opened.";
}

bool SerialPort::isOpen()
{
	return mSerialPort.is_open();
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


void SerialPort::setReadCallback( msgCallbackFcn fcn  )
{
	mMessageHandler = fcn;
	return;
}

void SerialPort::setWriteCallback(  timeoutHandlerFcn fcn  )
{
	mWriteCallback = fcn;
	return;
}
void SerialPort::setReadTimeoutCallback(  timeoutHandlerFcn fcn  )
{
	mReadTimeoutHandler = fcn;
	return;
}

void SerialPort::setLogLevel(log_level lvl)
{
	log.setModuleLogLevel(lvl);
	asioLog.setModuleLogLevel(lvl);
}

/*Close the serial port*/
void SerialPort::doClose( const boost::system::error_code& error )
{
	if( error )
	{
		LOG_ERROR(asioLog, "Close: "+ error.message() );
		throw std::runtime_error(error.message());
	}
	if( error == boost::asio::error::operation_aborted )
	{
		return;
	}
	mTimeoutWrite.cancel();
	mSerialPort.close();
	LOG_ERROR(asioLog, "Serial Port closed");
	return;
}
QGS_ModuleMsgBase::ptr SerialPort::write(QGS_ModuleMsgBase::ptr msg, AppLog &logger)
{
	if( ! mSerialPort.is_open() )
	{
		LOG_ERROR(logger, "Port is not open." );
		return msg;
	}
	if(mWriteOngoing)
	{
		//error
		LOG_ERROR(logger, "Write called during ongoing write operation." );
	}
	//Package into a slip packet and send the packaged data.
	BinaryOStream os;
	if(msg)
	{
		os << *msg; // All messages are serializable to the BinaryOStream via base class.
	}

	SlipPacket tmpSlip = SlipPacket(os.get_internal_vec(), true);
	std::size_t packetSize = tmpSlip.GetPacket().size();
	LOG_MESSAGE_TRACE(logger, "Writing " << packetSize << " bytes");
	mTimeoutWrite.expires_from_now( boost::posix_time::milliseconds( 1000 ) );
	mTimeoutWrite.async_wait(
			boost::bind( & SerialPort::timerWriteCallback,
					shared_from_this(),
					boost::asio::placeholders::error ) );

	boost::asio::async_write( mSerialPort,boost::asio::buffer( &tmpSlip.GetPacket().front(), tmpSlip.GetPacket().size()),
			boost::bind( & SerialPort::writeCallback, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) );
	return std::move(msg);
}



/*Start an async read operation.*/
void SerialPort::read()
{
	if( ! mSerialPort.is_open() )
	{
		return;
	}
	boost::asio::async_read_until( mSerialPort,
			mStreambuf,
			SlipPacket::SlipControlOctets::frame_boundary_octet,
			std::bind( &SerialPort::readCallback, this, std::placeholders::_1, std::placeholders::_2)) ;
}

bool SerialPort::ready()
{
	return !mWriteOngoing;
}

/*Callback from write operation.*/
void SerialPort::writeCallback( const boost::system::error_code& error,
		std::size_t bytes_transferred )
{
	mWriteOngoing = false;
	try
	{
		if( error )
		{
			LOG_ERROR(asioLog, "Write_callback called with an error: " + error.message() );
			doClose( error );
		}
		mTimeoutWrite.cancel();
	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(log, e.what() );
	}
	if(mWriteCallback)
	{
		mWriteCallback();
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
			doClose( error );
			return;
		}

		std::string data{
			buffers_begin(mStreambuf.data()),
			buffers_begin(mStreambuf.data()) + bytes_transferred};
		mStreambuf.consume(bytes_transferred);
		mReceivedData.append(data);
		LOG_MESSAGE_TRACE(asioLog, "Read  Callback: length = " << mReceivedData.length());
		if(mReceivedData.length() < 4)
		{
			read();
			return;
		}
		
		SlipPacket tmpSlip((const uint8_t*)mReceivedData.c_str(), mReceivedData.length(), false);
		
		mReceivedData.erase();
		QGS_ModuleMsgBase::ptr msg = mParser.parse(std::vector<unsigned char>(tmpSlip.GetPayload()));
		if(!msg)
		{
			LOG_ERROR(asioLog, " Read_callback failed to parse the message." );
			read();
			return;
		}
		if(msg->getType() == messageTypes_t::Msg_Transmission_e)
		{
			LOG_DEBUG(asioLog, "Received OK/NOK nr: " << (int)msg->getMsgNr());
		}
		if(msg) // do not use tmpSlip after move!
		{
			if(mMessageHandler)
			{
				mMessageHandler(std::move(msg), asioLog);
			}
		}
		else
		{
			LOG_ERROR(asioLog,  "Parser error");
			if(mMessageHandler)
			{

				mMessageHandler(NULL, asioLog);
			}
		}

	}
	catch (const std::runtime_error& e)
	{
		LOG_ERROR(asioLog, e.what() );
		mReceivedData.erase();
	}
	read();
}

/* Called when the write timer's deadline expire */
void SerialPort::timerWriteCallback( const boost::system::error_code& error )
{
	mWriteOngoing = false;
	if( error )
	{
		/*If the timer was called with the operation_aborted error code then
		 * the timer was canceled and did not fire. Everything else is an error*/
		if( error != boost::asio::error::operation_aborted )
		{
			LOG_ERROR(asioLog, "Write timer callback: " << error.message());
			doClose( error );
			throw std::runtime_error(error.message());
		}

		return;
	}
	LOG_ERROR(asioLog, "Write timeout fired." );
	mSerialPort.cancel(); // Close all asynchronous operation with serial port.
	//will cause read_callback to fire with an error
	return;
}

SerialPort::~SerialPort()
{
	boost::system::error_code error;
	mSerialPort.cancel( error );
	mSerialPort.close( error );
	if( error )
	{
		LOG_ERROR(log, "Error closing the serial port: " + error.message());
	}
}

} /* namespace QuadGS */

