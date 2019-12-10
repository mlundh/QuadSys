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

#include <boost/algorithm/string.hpp>
#include <string>
#include "msg_enums.h"
#include "msgAddr.h"
#include "SlipPacket.h"

#include "QGS_Msg.h"
#include "QGS_Module.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"
#include "Msg_Transmission.h"
#include "Msg_TestTransmission.h"
#include "Msg_Display.h"
#define NR_RETRIES (2)
namespace QuadGS {


Serial_Manager::Serial_Manager(msgAddr_t name, msgAddr_t transmissionAddr)
:QGS_MessageHandlerBase(name)
,mIo_service()
,mWork(new boost::asio::io_service::work(mIo_service))
,mTimeoutRead(mIo_service)
,mThread_io(NULL)
,mOutgoingFifo(10)
,mRetries(0)
,mOngoing()
,mTransmissionAddr(transmissionAddr)
{
	mCommands.push_back(UiCommand("serialSetBaudRate","Set the baud rate of the serial port.",std::bind(&Serial_Manager::setBaudRateCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("serialSetFlowControl","Set the flow control of the serial port.",std::bind(&Serial_Manager::setFlowControlCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("serialSetParity","Set the parity of the serial port.",std::bind(&Serial_Manager::setParityCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("serialSetStopBits","Set the number of stop bits used. ",std::bind(&Serial_Manager::setStopBitsCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("serialOpenPort","Open the serial port.",std::bind(&Serial_Manager::openCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("serialStartReadPort","Start the read operation.",std::bind(&Serial_Manager::startReadCmd, this, std::placeholders::_1)));
	mCommands.push_back(UiCommand("serialTest","Test the serial connection.",std::bind(&Serial_Manager::testSerial, this, std::placeholders::_1)));
	setPortFcn(std::bind(&Serial_Manager::ReceivingFcnIo, this, std::placeholders::_1));
	initialize();
}



Serial_Manager::~Serial_Manager()
{
	mWork.reset();
	mPort.reset();
	mIo_service.stop();
	if(mThread_io && mThread_io->joinable())
	{
		mThread_io->join();
	}
}

void Serial_Manager::initialize()
{
	mThread_io = new std::thread(std::bind(&Serial_Manager::runThread, this));
	mPort = QuadGS::SerialPort::create(mIo_service);
	mPort->setReadCallback( std::bind(&Serial_Manager::messageHandler, this, std::placeholders::_1) );
	mPort->setWriteCallback( std::bind(&Serial_Manager::writeCallback, this) );

}

void Serial_Manager::startRead()
{
	mPort->read();
}

std::string Serial_Manager::openCmd(std::string path)
{
	std::string returnStr;
	if(mPort->isOpen())
	{
		returnStr = "Port already open.";
	}
	else
	{
		returnStr = mPort->open( path );
		startRead();
		doWrite();
	}

	return returnStr;
}

std::string Serial_Manager::setBaudRateCmd( std::string baud )
{
	int baudInt = std::stoi(baud);
	mPort->setBaudRate(baudInt);
	return "Baud rate changed.";
}

std::string Serial_Manager::setFlowControlCmd(  std::string flow_ctrl )
{
	int flowCtrl = std::stoi(flow_ctrl);
	mPort->setFlowControl(static_cast<b_a_sp::flow_control::type>(flowCtrl));
	return "Flow control changed.";
}

std::string Serial_Manager::setParityCmd( std::string parity )
{
	int par = std::stoi(parity);
	mPort->setParity(static_cast<b_a_sp::parity::type>(par));
	return "Parity changed.";
}

std::string Serial_Manager::setStopBitsCmd( std::string stop_bits )
{
	int stopBits = std::stoi(stop_bits);
	mPort->setStopBits(static_cast<b_a_sp::stop_bits::type>(stopBits));
	return "Stop bits changed.";
}

std::string Serial_Manager::startReadCmd(std::string)
{
	startRead();
	return "Reading from port.";
}

std::string Serial_Manager::testSerial(std::string path)
{
	Msg_TestTransmission::ptr msg = std::make_unique<Msg_TestTransmission>(msgAddr_t::FC_SerialIOrx_e, 0xDEADBEEF, "Test string\0");
	std::string result = "Test message sent!";
	sendMsg(std::move(msg));
	return result;
}


std::string Serial_Manager::getStatus()
{
	std::string s("N/A");
	return s;
}


void Serial_Manager::process(Msg_GetUiCommands* message)
{
	for(UiCommand command : mCommands)
	{
		Msg_RegUiCommand::ptr ptr = std::make_unique<Msg_RegUiCommand>(message->getSource(), command.command, command.doc);
		sendMsg(std::move(ptr));
	}
}
void Serial_Manager::process(Msg_FireUiCommand* message)
{
	for(UiCommand command : mCommands)
	{
		if(command.command.compare(message->getCommand()) == 0)
		{
			std::string result = command.function(message->getArgs());
			Msg_UiCommandResult::ptr ptr = std::make_unique<Msg_UiCommandResult>(message->getSource(), result);
			sendMsg(std::move(ptr));
			return;
		}
	}

	Msg_UiCommandResult::ptr ptr = std::make_unique<Msg_UiCommandResult>(message->getSource(), "No such command.");
	sendMsg(std::move(ptr));
	return;
}
void Serial_Manager::process(Msg_TestTransmission* message)
{
	bool result = true;
	if(message->getTest() != 1)
	{
		result = false;
	}
	if(strcmp(message->getPayload().c_str(), "Test OK") != 0)
	{
		result = false;
	}
	std::string resultStr = result ? "Pass":"Fail";
	std::string display = "Test result: " + resultStr;
	Msg_Display::ptr dispMsg = std::make_unique<Msg_Display>(GS_GUI_e, display);
	sendMsg(std::move(dispMsg));
}


void Serial_Manager::ReceivingFcnIo(std::unique_ptr<QGS_ModuleMsgBase> message)
{
	mOutgoingFifo.push(std::move(message));
	doWrite();
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
		mLogger.QuadLog(severity_level::error, "Exception from  io service: " + exception );
	}
	catch(...)
	{
		mLogger.QuadLog(severity_level::error, "Unknown exception from  io service! " );
	}
}

void Serial_Manager::timeoutHandler()
{

}

void Serial_Manager::messageHandler(QGS_ModuleMsgBase::ptr msg)
{
	doWrite();
	if(!msg)
	{
		//Received NOK message, trigger a re-try!
		Msg_Transmission::ptr transmission = std::make_unique<Msg_Transmission>(mTransmissionAddr, transmission_NOK);
		sendMsg(std::move(transmission));
		return;
	}	
	if(msg->getType() == messageTypes_t::Msg_Transmission_e)
	{
		std::stringstream ss;
		ss << "Received OK/NOK msg nr: " << (int)msg->getMsgNr();
		mLogger.QuadLog(severity_level::message_trace, ss.str());

		mTimeoutRead.cancel();
		Msg_Transmission* transMsg = dynamic_cast<Msg_Transmission*>(msg.get());
		
		if(mMsgNrTx != transMsg->getMsgNr())
		{
			std::stringstream ss;
			ss << "Ack does not match expected message number: " << (int)transMsg->getMsgNr() << " expected: " << (int) mMsgNrTx;
			mLogger.QuadLog(QuadGS::error, ss.str());
		}

		if(transMsg->getStatus() == transmission_OK) 
		{
			// Transmission ok, discard the outgoing message, no need to save after successful transmission. Increase the messageNumber.
			mOngoing.release(); // we got a transmissionOK message, this is the end of an ongoing transmission.
			mRetries = 0;
			msg->setMsgNr((mMsgNrTx++)%256);

		}
		else if(transMsg->getStatus() == transmission_NOK)
		{
			if(mRetries < NR_RETRIES)
			{
				mRetries++;
				std::stringstream ss;
				ss << "Transmission failed, resending msg nr: " << (int)transMsg->getMsgNr();
				mLogger.QuadLog(severity_level::warning, ss.str());
			}
			else
			{
				mOngoing.release(); 
				mRetries = 0;
				mLogger.QuadLog(severity_level::error, "Transmission failed, droping package.");
			}

		}
	}
	// Respond transmission OK and forward the message.
	else
	{
		std::stringstream ss;
		ss << "Received msg nr: " << (int)msg->getMsgNr();
		mLogger.QuadLog(severity_level::message_trace, ss.str());

		//Received message OK, return transmission OK!
		Msg_Transmission::ptr transmission = std::make_unique<Msg_Transmission>(mTransmissionAddr, transmission_OK);
		transmission->setMsgNr(msg->getMsgNr());

		sendMsg(std::move(transmission));

		// If we get a broadcast message, we should only handle it internally.
		if(msg->getDestination() == msgAddr_t::Broadcast_e)
		{
			msg->setDestination(msgAddr_t::GS_Broadcast_e); // TODO make this dynamic based on name...
		}
		// then send to the router.
		sendExternalMsg(std::move(msg));
	}
}
void Serial_Manager::doWrite()
{
	if(!mPort->isOpen())
	{
		mLogger.QuadLog(severity_level::error, "Port not open.");
		return;
	}
	if(mOngoing || mOutgoingFifo.empty() )
	{
		return;
	}
	else if(!mPort->ready())
	{
		mLogger.QuadLog(severity_level::error, "Port not ready.");
		return;
	}

	QGS_ModuleMsgBase::ptr msg = mOutgoingFifo.dequeue();

	if(msg->getType() != Msg_Transmission_e)
	{
		msg->setMsgNr(mMsgNrTx);
	}
	if(msg->getType() == messageTypes_t::Msg_Transmission_e)
	{
		std::stringstream ss;
		ss << "Transmitting OK/NOK nr: " << (int)msg->getMsgNr();
		mLogger.QuadLog(severity_level::message_trace, ss.str());
	}
	else
	{
		std::stringstream ss;
		ss << "Transmitting msg nr: " << (int)msg->getMsgNr();
		mLogger.QuadLog(severity_level::message_trace, ss.str());
	}	// Do not expect a result, and do not re-transmitt a transmission message.



	if(msg->getType() != Msg_Transmission_e)
	{
		startReadTimer();
		mOngoing = mPort->write( std::move(msg) );
	}
	else
	{
		// Transmissions should not be re-sent.
		mPort->write( std::move(msg) );
	}
}

void Serial_Manager::writeCallback()
{
	mLogger.QuadLog(severity_level::message_trace, "Write callback.");

	doWrite();
}

void Serial_Manager::startReadTimer(int timeout)
{
	mTimeoutRead.expires_from_now( boost::posix_time::milliseconds( timeout ) );
	mTimeoutRead.async_wait(
			boost::bind( & Serial_Manager::timerReadCallback,
					this,
					boost::asio::placeholders::error ) );
}

/* Called when the read timer's deadline expire. Do not cancel payload read, it is still used. */
void Serial_Manager::timerReadCallback( const boost::system::error_code& error )
{
	if( error )
	{
		/*If the timer was called with the operation_aborted error code then
		 * the timer was canceled and did not fire. Everything else is an error*/
		if( error != boost::asio::error::operation_aborted )
		{
			mLogger.QuadLog(severity_level::error, "Read timer callback: " + error.message());
			throw std::runtime_error(error.message());
		}
		return;
	}
	else
	{
		mLogger.QuadLog(severity_level::warning, " Read timeout fired.");
	}

	if(mRetries < NR_RETRIES) 
	{
		mRetries++;
		mLogger.QuadLog(severity_level::warning, "No reply, retrying");
		startReadTimer();
		mOngoing = mPort->write( std::move(mOngoing) ); // resend.

	}
	else
	{
		mRetries = 0;
		mOngoing.release();
		mLogger.QuadLog(severity_level::error, "Transmission failed: timeout.");
		doWrite();// Poll doWrite to see if there is more to be written.
	}

	return;
}



} /* namespace QuadGS */
