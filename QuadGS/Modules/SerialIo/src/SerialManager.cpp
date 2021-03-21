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

#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include "msg_enums.h"
#include "msgAddr.h"
#include "SlipPacket.h"

#include "QGS_Msg.h"
#include "QGS_Module.h"
#include "Msg_RegUiCommand.h"
#include "Msg_UiCommandResult.h"
#include "Msg_TestTransmission.h"
#include "Msg_Display.h"
#define NR_RETRIES (2)
namespace QuadGS
{

	Serial_Manager::Serial_Manager(msgAddr_t name, msgAddr_t transmissionAddr)
		: QGS_MessageHandlerBase(name), mPortReady(), mIo_service(), mWork(new boost::asio::io_service::work(mIo_service)), mTimeoutRead(mIo_service), mThread_io(NULL), mThreadWriter(NULL), mOutgoingFifo(40), mTransmissionAddr(transmissionAddr), mTransmittDone(), mTxSuccess(false), mRetries(0), writerLog("WriterThread", std::bind(&Serial_Manager::sendMsg, this, std::placeholders::_1))
	{
		mCommands.push_back(UiCommand("serialSetBaudRate", "Set the baud rate of the serial port.", std::bind(&Serial_Manager::setBaudRateCmd, this, std::placeholders::_1)));
		mCommands.push_back(UiCommand("serialSetFlowControl", "Set the flow control of the serial port.", std::bind(&Serial_Manager::setFlowControlCmd, this, std::placeholders::_1)));
		mCommands.push_back(UiCommand("serialSetParity", "Set the parity of the serial port.", std::bind(&Serial_Manager::setParityCmd, this, std::placeholders::_1)));
		mCommands.push_back(UiCommand("serialSetStopBits", "Set the number of stop bits used. ", std::bind(&Serial_Manager::setStopBitsCmd, this, std::placeholders::_1)));
		mCommands.push_back(UiCommand("serialOpenPort", "Open the serial port.", std::bind(&Serial_Manager::openCmd, this, std::placeholders::_1)));
		mCommands.push_back(UiCommand("serialStartReadPort", "Start the read operation.", std::bind(&Serial_Manager::startReadCmd, this, std::placeholders::_1)));
		mCommands.push_back(UiCommand("serialTest", "Test the serial connection.", std::bind(&Serial_Manager::testSerial, this, std::placeholders::_1)));
		setPortFcn(std::bind(&Serial_Manager::ReceivingFcnIo, this, std::placeholders::_1));
		initialize();
		setProcessingFcn(std::bind(&Serial_Manager::processingFcn, this));
		startProcessing();
	}

	Serial_Manager::~Serial_Manager()
	{
		stopProcessing();
		mOutgoingFifo.stop();
		mStop = true;
		mTransmittDone.stop();
		mPortReady.stop();
		mWork.reset();
		mPort.reset();
		mIo_service.stop();
		if (mThread_io && mThread_io->joinable())
		{
			mThread_io->join();
			delete mThread_io;
		}
		if (mThreadWriter && mThreadWriter->joinable())
		{
			mThreadWriter->join();
			delete mThreadWriter;
		}
	}

	void Serial_Manager::initialize()
	{
		mThread_io = new std::thread(std::bind(&Serial_Manager::runThread, this));
		mPort = QuadGS::SerialPort::create(mIo_service, std::bind(&Serial_Manager::sendMsg, this, std::placeholders::_1));
		mPort->setReadCallback(std::bind(&Serial_Manager::messageHandler, this, std::placeholders::_1, std::placeholders::_2));
		mPort->setWriteCallback(std::bind(&Serial_Manager::writeCallback, this));
	}

	void Serial_Manager::processingFcn()
	{
		try
		{
			handleMessages(true);
		}
		catch (const std::exception &e)
		{
			LOG_ERROR(log, e.what());
		}
	}

	void Serial_Manager::startRead()
	{
		mPort->read();
	}

	std::string Serial_Manager::openCmd(std::string path)
	{
		std::string returnStr;
		if (mPort->isOpen())
		{
			returnStr = "Port already open.";
		}
		else
		{
			returnStr = mPort->open(path);
			startRead();
			mThreadWriter = new std::thread(std::bind(&Serial_Manager::doWrite, this)); // spawn a writer thread.
		}

		return returnStr;
	}

	std::string Serial_Manager::setBaudRateCmd(std::string baud)
	{
		int baudInt = std::stoi(baud);
		mPort->setBaudRate(baudInt);
		return "Baud rate changed.";
	}

	std::string Serial_Manager::setFlowControlCmd(std::string flow_ctrl)
	{
		int flowCtrl = std::stoi(flow_ctrl);
		mPort->setFlowControl(static_cast<b_a_sp::flow_control::type>(flowCtrl));
		return "Flow control changed.";
	}

	std::string Serial_Manager::setParityCmd(std::string parity)
	{
		int par = std::stoi(parity);
		mPort->setParity(static_cast<b_a_sp::parity::type>(par));
		return "Parity changed.";
	}

	std::string Serial_Manager::setStopBitsCmd(std::string stop_bits)
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

	void Serial_Manager::process(Msg_GetUiCommands *message)
	{
		for (UiCommand command : mCommands)
		{
			Msg_RegUiCommand::ptr ptr = std::make_unique<Msg_RegUiCommand>(message->getSource(), command.command, command.doc);
			sendMsg(std::move(ptr));
		}
	}
	void Serial_Manager::process(Msg_FireUiCommand *message)
	{
		for (UiCommand command : mCommands)
		{
			if (command.command.compare(message->getCommand()) == 0)
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

	void Serial_Manager::process(Msg_TestTransmission *message)
	{
		bool result = true;
		if (message->getTest() != 1)
		{
			result = false;
		}
		if (strcmp(message->getPayload().c_str(), "Test OK") != 0)
		{
			result = false;
		}
		std::string resultStr = result ? "Pass" : "Fail";
		std::string display = "Test result: " + resultStr;
		Msg_Display::ptr dispMsg = std::make_unique<Msg_Display>(GS_GUI_e, display);
		sendMsg(std::move(dispMsg));
	}

	void Serial_Manager::process(Msg_Transmission *transMsg)
	{

		mTimeoutRead.cancel();
		if (mMsgNrTx != transMsg->getMsgNr())
		{
			LOG_ERROR(log, "Ack does not match expected message number: " << (int)transMsg->getMsgNr() << " expected: " << (int)mMsgNrTx);
		}
		if (transMsg->getStatus() == transmission_OK)
		{
			mTxSuccess = true;
			mTransmittDone.notify();
		}
		else
		{
			mTxSuccess = false;
			mRetries++;
			mTransmittDone.notify();
		}
	}

	void Serial_Manager::process(Msg_GsLogLevel *message)
	{
		uint32_t logLevelInt = message->getLoglevel();
		if (logLevelInt >= log_level::invalid && logLevelInt <= log_level::message_trace)
		{
			log_level logLevel = static_cast<log_level>(logLevelInt);
			mPort->setLogLevel(logLevel);
			writerLog.setModuleLogLevel(logLevel);
		}
		QGS_LoggingModule::process(message);
	}

	void Serial_Manager::ReceivingFcnIo(std::unique_ptr<QGS_ModuleMsgBase> message)
	{
		try
		{
			mOutgoingFifo.push(std::move(message));
		}
		catch (const std::exception &e)
		{
			LOG_ERROR(log, e.what());
		}
	}

	void Serial_Manager::runThread()
	{
		try
		{
			mIo_service.run();
		}
		catch (const std::exception &exc)
		{
			std::string exception(exc.what());
			LOG_MESSAGE_TRACE(log, "Exception from  io service: " << exception);
		}
		catch (...)
		{
			LOG_ERROR(log, "Unknown exception from  io service! ");
		}
	}

	void Serial_Manager::timeoutHandler()
	{
	}

	// excecuted in the io-service thread.
	void Serial_Manager::messageHandler(QGS_ModuleMsgBase::ptr msg, AppLog &logger)
	{
		if (!msg)
		{
			//Received NOK message, trigger a re-try!
			Msg_Transmission::ptr transmission = std::make_unique<Msg_Transmission>(mTransmissionAddr, transmission_NOK);
			sendMsg(std::move(transmission));
			return;
		}
		if (msg->getType() == messageTypes_t::Msg_Transmission_e)
		{
			LOG_MESSAGE_TRACE(logger, "Received OK/NOK msg nr: " << (int)msg->getMsgNr());
		}
		// Respond transmission OK and forward the message.
		else
		{
			LOG_MESSAGE_TRACE(logger, "Received msg nr: " << (int)msg->getMsgNr());

			//Received message OK, return transmission OK!
			Msg_Transmission::ptr transmission = std::make_unique<Msg_Transmission>(mTransmissionAddr, transmission_OK);
			transmission->setMsgNr(msg->getMsgNr());

			sendMsg(std::move(transmission));
		}
		// If we get a broadcast message, we should only handle it internally.
		if (msg->getDestination() == msgAddr_t::Broadcast_e)
		{
			msg->setDestination(msgAddr_t::GS_Broadcast_e); // TODO make this dynamic based on name...
		}
		// then send to the router.
		sendExternalMsg(std::move(msg));
	}

	// Executed in its own thread. Will write what is in the outgoing FIFO to the serial port.
	void Serial_Manager::doWrite()
	{
		if (!mPort->isOpen())
		{
			LOG_ERROR(writerLog, "Port not open.");
			return;
		}

		while (!mStop)
		{
			QGS_ModuleMsgBase::ptr msg = mOutgoingFifo.dequeue();
			if (!msg)
			{
				break;
			}
			// Do not expect a result, and do not re-transmitt a transmission message.
			if (msg->getType() != Msg_Transmission_e)
			{
				mMsgNrTx = ((mMsgNrTx + 1) % 256);

				msg->setMsgNr(mMsgNrTx);
				LOG_MESSAGE_TRACE(writerLog, "Transmitting msg nr: " << (int)msg->getMsgNr());
				startReadTimer();
				msg = mPort->write(std::move(msg), writerLog);

				mTransmittDone.wait();
				while (!mTxSuccess && (mRetries < NR_RETRIES))
				{
					msg->setMsgNr(mMsgNrTx);
					startReadTimer();
					msg = mPort->write(std::move(msg), writerLog);

					LOG_WARNING(writerLog, "Transmission failed, resending msg nr: " << (int)msg->getMsgNr());

					mTransmittDone.wait();
				}
				if (!mTxSuccess)
				{
					LOG_WARNING(writerLog, "Transmission failed, dropping msg nr: " << (int)msg->getMsgNr());
				}
			}
			else
			{
				LOG_MESSAGE_TRACE(writerLog, "Transmitting OK/NOK nr: " << (int)msg->getMsgNr());

				// Transmissions should not be re-sent.
				mPort->write(std::move(msg), writerLog);
			}
			mPortReady.wait();
		}
	}

	void Serial_Manager::writeCallback()
	{
		mPortReady.notify();
	}

	void Serial_Manager::startReadTimer(int timeout)
	{
		mTimeoutRead.expires_from_now(boost::posix_time::milliseconds(timeout));
		mTimeoutRead.async_wait(
			boost::bind(&Serial_Manager::timerReadCallback,
						this,
						boost::asio::placeholders::error));
	}

	/* Called when the read timer's deadline expire. Do not cancel payload read, it is still used. */
	void Serial_Manager::timerReadCallback(const boost::system::error_code &error)
	{
		if (error)
		{
			/*If the timer was called with the operation_aborted error code then
		 * the timer was canceled and did not fire. Everything else is an error*/
			if (error != boost::asio::error::operation_aborted)
			{
				LOG_ERROR(log, "Read timer callback: " << error.message());
				throw std::runtime_error(error.message());
			}
			return;
		}
		else
		{
			mTransmittDone.notify();
			//mLogger.QuadLog(severity_level::warning, " Read timeout fired.");
			LOG_WARNING(log, " Read timeout fired.");
		}

		//TODO add thread saftey measures!
		mRetries++;

		return;
	}

} /* namespace QuadGS */
