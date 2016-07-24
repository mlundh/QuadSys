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
#include "IoBase.h"
#include "Core.h"
#include "TimedFifo.hpp"
#include "Log.h"


using namespace boost::asio;
namespace QuadGS {

/**
 * @class Serial_Manager
 *
 * Class handling the serial port. Implements the IoBase, and can thus
 * be connected to a Core object.
 */
class Serial_Manager:
          public IoBase
{
public:

    virtual ~Serial_Manager();

    /**
     * Start the serial manager. This creates and starts the internal handler threads.
     */
    void Start();

    /**
     * Create an instance of the Serial_Manager class.
     * @return  A pointer to the instance.
     */
    static IoBase* create();

    /**
     * Write function used by users of the Io module.
     * @param ptr   The QSP that should be sent.
     */
    virtual void write( QspPayloadRaw::Ptr ptr);

    /**
     * Start the read operation. Can be used as soon as the serial
     * port has been opened.
     */
    virtual void startRead( void );

    /**
     * Set a read callback. The callback will get called when the
     * I/O module has received a complete QSP.
     * @param fcn   The callback function.
     */
    virtual void setReadCallback( IoBase::MessageHandlerFcn fcn );

    /**
     * Get the user commands available for the module.
     * @return  A vector of commands.
     */
    virtual std::vector<Command::ptr> getCommands( );

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


private:
    Serial_Manager();

    /**
     * Method that is run in a separate thread. Will handle all async
     * operations from the serial port.
     */
    void RunThread();

    /**
     * Handler for timeouts. Will get called when there is no response from
     * the remote. Will resend the current message up to three times.
     */
    void timeoutHandler();

    /**
     * Handler function for a complete message. Will pass the message to any
     * registered message handler. Also handles logging of messages.
     * @param ptr   The received message.
     */
    void messageHandler( QspPayloadRaw::Ptr ptr);

    /**
     * Internal write message. This is the only function allowed to write to the
     * serial port. Also handles logging of messages.
     */
    void doWrite();



    QuadGS::SerialPort::ptr mPort;
    boost::asio::io_service mIo_service;
    std::unique_ptr<boost::asio::io_service::work> mWork;
    std::thread *mThread_io;
    TimedFifo<QspPayloadRaw::Ptr> mFifo;
    IoBase::MessageHandlerFcn mMessageHandler;
    int mRetries;
    bool mOngoing;
    Log mLog;


};

} /* namespace QuadGS */
#endif /* SERIALMANAGER_H_ */
