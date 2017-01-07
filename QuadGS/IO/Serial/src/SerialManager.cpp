/*
 * SerialManager.cpp
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
#include "SerialManager.h"

#include "SlipPacket.h"
#include <boost/algorithm/string.hpp>

#include "QCMsgHeader.h"
#include "QuadGSMsg.h"
#include "FcParser.h"

namespace QuadGS {
Serial_Manager::Serial_Manager():
        		mIo_service()
,mWork(new boost::asio::io_service::work(mIo_service))
,mThread_io(NULL)
,mOutgoingFifo(10, 1000)
,mRetries(0)
,mOngoing(false)
,mLog("SerialManager")
{
}

void Serial_Manager::start()
{
	mThread_io = new std::thread(std::bind(&Serial_Manager::runThread, this));
	mPort = QuadGS::SerialPort::create(mIo_service);
	mPort->setReadCallback( std::bind(&Serial_Manager::messageHandler, this, std::placeholders::_1, std::placeholders::_2) );
	mPort->setReadTimeoutCallback( std::bind(&Serial_Manager::timeoutHandler, this) );
	mPort->setParser(std::make_shared<FcParser>());

}


IoBase* Serial_Manager::create()
{
	IoBase* tmp = new Serial_Manager;
	Serial_Manager* manager = dynamic_cast<Serial_Manager*>(tmp);

	if(!manager)
	{
		throw std::runtime_error("Not able to create Serial Manager.");
	}
	manager->start();
	return tmp;
}

void Serial_Manager::startRead()
{
	mPort->read();
}

void Serial_Manager::write( QCMsgHeader::ptr header, QuadGSMsg::QuadGSMsgPtr payload)
{
	mOutgoingFifo.push(std::make_pair(header, payload));
	doWrite();
}

void Serial_Manager::setMessageCallback(  IoBase::MessageHandlerFcn fcn )
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
			std::bind(&Serial_Manager::setNameCmd, this, std::placeholders::_1),
			"Set the port name.", Command::ActOn::File));
	Commands.push_back(std::make_shared<Command> ("setBaudRate",
			std::bind(&Serial_Manager::setBaudRateCmd, this, std::placeholders::_1),
			"Set the baud rate of the port, default 57600 baud.", Command::ActOn::IO));
	Commands.push_back(std::make_shared<Command> ("setFlowControl",
			std::bind(&Serial_Manager::setFlowControlCmd, this, std::placeholders::_1),
			"Set flow control, default None.", Command::ActOn::IO));
	Commands.push_back(std::make_shared<Command> ("setParity",
			std::bind(&Serial_Manager::setParityCmd, this, std::placeholders::_1),
			"Set parity, default none.", Command::ActOn::IO));
	Commands.push_back(std::make_shared<Command> ("setStopBits",
			std::bind(&Serial_Manager::setStopBitsCmd, this, std::placeholders::_1),
			"Set number of stop bits to be used, default one.", Command::ActOn::IO));
	Commands.push_back(std::make_shared<Command> ("openPort",
			std::bind(&Serial_Manager::openCmd, this, std::placeholders::_1),
			"Open the serial port.", Command::ActOn::File));
	Commands.push_back(std::make_shared<Command> ("startReadPort",
			std::bind(&Serial_Manager::startReadCmd, this, std::placeholders::_1),
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

void Serial_Manager::runThread()
{
	try
	{
		mIo_service.run();
	}
	catch(const std::exception &exc)
	{
		std::string exception(exc.what());
		mLog.QuadLog(severity_level::error, "Exception from  io service: " + exception );
	}
	catch(...)
	{
		mLog.QuadLog(severity_level::error, "Unknown exception from  io service! " );
	}
}

void Serial_Manager::timeoutHandler()
{
	if(mRetries < 2)
	{
		mRetries++;
		mLog.QuadLog(severity_level::warning, "No reply, retrying");

	}
	else
	{
		mRetries = 0;
		mOutgoingFifo.pop();
		mLog.QuadLog(severity_level::error, "Transmission failed.");
	}
	// Poll doWrite to see if there is more to be written.
	mOngoing = false;
	doWrite();
}

void Serial_Manager::messageHandler(QCMsgHeader::ptr header, QuadGSMsg::QuadGSMsgPtr payload)
{
	// Transmission ok, pop from fifo and set ok to send again.
	if(!mOutgoingFifo.empty())
	{
		mOutgoingFifo.pop();
	}
	mOngoing = false;

	// Log message.
	mLog.QuadLog(severity_level::message_trace, "Received: " + (header ? header->toString() : "")  + (payload?payload->toString():"") );

	doWrite();

	if( mMessageHandler )
	{
		mMessageHandler( header, payload );
	}

}
void Serial_Manager::doWrite()
{
	if(mOngoing)
	{
		return;
	}
	if(mOutgoingFifo.empty())
	{
		mLog.QuadLog(severity_level::warning, "doWrite called without anything to write.");
		return;
	}
	QCMsgHeader::ptr header = mOutgoingFifo.front().first;
	QuadGSMsg::QuadGSMsgPtr payload = mOutgoingFifo.front().second;
	mLog.QuadLog(severity_level::message_trace, "Transmitting: " + (header ? header->toString() : "")  + (payload?payload->toString():""));

	mOngoing = true;
	mPort->write( header, payload );
}




} /* namespace QuadGS */
