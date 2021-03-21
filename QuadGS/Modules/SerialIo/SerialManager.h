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
#include "Semaphore.h"

#include "QGS_Module.h"
#include "Msg_GetUiCommands.h"
#include "Msg_FireUiCommand.h"
#include "Msg_Test.h"
#include "Msg_TestTransmission.h"
#include "Msg_Transmission.h"


using namespace boost::asio;
namespace QuadGS {

/**
 * @class Serial_Manager
 *
 * Class handling the serial port. Implements the IoBase, and can thus
 * be connected to a Core object.
 */
class Serial_Manager
		: public QGS_PortModule
		, public QGS_MessageHandler<Msg_GetUiCommands>
		, public QGS_MessageHandler<Msg_FireUiCommand>
		, public QGS_MessageHandler<Msg_TestTransmission>
		, public QGS_MessageHandler<Msg_Transmission>

{

public:

    Serial_Manager(msgAddr_t name, msgAddr_t transmissionAddr);


    virtual ~Serial_Manager();

    /**
     * Start the serial manager. This creates and starts the internal handler threads.
     */
    virtual void initialize();

    /**
     * @brief Processing function for the threaded module. Handles incoming messages from the router.
     * 
     * Module thread scope. 
     */
    void processingFcn();

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
    std::string testSerial(std::string path);

    /**
     * @brief Processing functions. Processes all messages.
     * 
     * Module thread scope.
     *  
     */
	virtual void process(Msg_GetUiCommands* message);
	virtual void process(Msg_FireUiCommand* message);
	virtual void process(Msg_TestTransmission* message);
    virtual void process(Msg_Transmission* message);
    virtual void process(Msg_GsLogLevel* message);

    /**
     * @brief Port function. Receives the messages from the Router. Re-entrant.
     * 
     * @param message 
     * 
     * Router thread scope.
     */
	void ReceivingFcnIo(std::unique_ptr<QGS_ModuleMsgBase> message);

private:

    /**
     * Method that is run in a separate thread. Will handle all async
     * operations from the serial port.
     */
    void runThread();

    /**
     * Handler for timeouts. Will get called when there is no response from
     * the remote. Will resend the current message up to three times.
     * 
     * io service thread scope.
     */
    void timeoutHandler();

    /**
     * The serial port will call this.
     * 
     * IO service thread scope.
     * 
     * @param msg   The received message.
     */
    void messageHandler(QGS_ModuleMsgBase::ptr msg, AppLog &logger);

    /**
     * Internal write message. This is the only function allowed to write to the
     * serial port. Also handles logging of messages.
     * 
     * mThread_Writer thread scope.
     */
    void doWrite();

    /**
     * @brief Callback function for write. 
     * 
     * IO service thread scope.
     */
    void writeCallback();


    /**
     * Start the read timer. If timeout occurs then the read will be considered failed.
     * @param timeout
     * 
     * 
     */
   void startReadTimer(int timeout = 1000);


   /**
    * @brief read timeout timer callback. 
    * 
    * IO service thread scope.
    * 
    * @param error
    */
   void timerReadCallback( const boost::system::error_code& error );

	std::vector<UiCommand> mCommands;
    QuadGS::SerialPort::ptr mPort;
    Semaphore mPortReady;
    boost::asio::io_service mIo_service;
    std::unique_ptr<boost::asio::io_service::work> mWork;
    boost::asio::deadline_timer mTimeoutRead;
    std::thread *mThread_io;
    std::thread *mThreadWriter;
    ThreadSafeFifo<QGS_ModuleMsgBase::ptr> mOutgoingFifo;
    uint8_t mMsgNrTx = 0;
    msgAddr_t mTransmissionAddr;
    Semaphore mTransmittDone;
    std::atomic<bool> mTxSuccess;
    int mRetries;
    bool mStop = false;
    AppLog writerLog;


};

} /* namespace QuadGS */
#endif /* SERIALMANAGER_H_ */
