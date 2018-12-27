/*
 * SerialManager.h
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

#ifndef SERIALMANAGER_H_
#define SERIALMANAGER_H_

#include <boost/asio.hpp>
#include <stdio.h>
#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <functional>

#include "SerialPort.h"
#include "ThreadSafeFifo.hpp"

#include "QGS_Module.h"
#include "Msg_GetUiCommands.h"
#include "Msg_FireUiCommand.h"
#include "Msg_IoWrapper.h"


using namespace boost::asio;
namespace QuadGS {

/**
 * @class Serial_Manager
 *
 * Class handling the serial port. Implements the IoBase, and can thus
 * be connected to a Core object.
 */
class Serial_Manager
		: public QGS_ThreadedModule
		, public QGS_MessageHandler<Msg_GetUiCommands>
		, public QGS_MessageHandler<Msg_FireUiCommand>
		, public QGS_MessageHandler<Msg_IoWrapper>

{
	typedef std::function<std::string(std::string) > UiFcn;
	class UiCommand
	{
	public:
		UiCommand(std::string command,  std::string doc, UiFcn function);
		std::string command;
		std::string doc;
		UiFcn function;
	};
public:

    Serial_Manager(std::string name);


    virtual ~Serial_Manager();

    /**
     * Start the serial manager. This creates and starts the internal handler threads.
     */
    virtual void initialize();

    /**
     * Start the read operation. Can be used as soon as the serial
     * port has been opened.
     */
    virtual void startRead( void );

    /**
     * Get the current status of the io link.
     * @return  Status represented as a string.
     */
    virtual std::string getStatus();

    /**
     * User input commands.
     */
    std::string openCmd(std::string path);
    std::string setNameCmd( std::string port_name );
    std::string setBaudRateCmd( std::string baud );
    std::string setFlowControlCmd(  std::string flow_ctrl );
    std::string setParityCmd( std::string parity );
    std::string setStopBitsCmd( std::string stop_bits );
    std::string startReadCmd(std::string);

	virtual void process(Msg_GetUiCommands* message);
	virtual void process(Msg_FireUiCommand* message);
	virtual void process(Msg_IoWrapper* message);


private:

    /**
     * Method that is run in a separate thread. Will handle all async
     * operations from the serial port.
     */
    void runThread();

    /**
     * Handler for timeouts. Will get called when there is no response from
     * the remote. Will resend the current message up to three times.
     */
    void timeoutHandler();

    /**
     * The serial port will call this  TODO!!!
     * @param msg   The received message.
     */
    void messageHandler(QGS_ModuleMsgBase::ptr msg);

    /**
     * Internal write message. This is the only function allowed to write to the
     * serial port. Also handles logging of messages.
     */
    void doWrite();


	std::vector<UiCommand> mCommands;
    QuadGS::SerialPort::ptr mPort;
    boost::asio::io_service mIo_service;
    std::unique_ptr<boost::asio::io_service::work> mWork;
    std::thread *mThread_io;
    ThreadSafeFifo<QGS_ModuleMsgBase::ptr>mOutgoingFifo;
    int mRetries;
    bool mOngoing;
    uint8_t mMsgNr = 0;


};

} /* namespace QuadGS */
#endif /* SERIALMANAGER_H_ */
