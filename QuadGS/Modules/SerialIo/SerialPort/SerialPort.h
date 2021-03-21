/*
 * SerialPort.h
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

#ifndef SERIALPORT_H_
#define SERIALPORT_H_
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <cstddef>
#include <memory>

#include "AppLog.h"
#include "QGS_ModuleMsg.h"
#include "Parser.h"

typedef boost::asio::serial_port b_a_sp;

namespace QuadGS {


/**
 * Forward declarations
 */
class SlipPacket;

/**
 * Class containing everything to do with serial communication
 * in this program.
 */
class SerialPort
        : public std::enable_shared_from_this< SerialPort >
{
	typedef std::function<void(QGS_ModuleMsgBase::ptr, AppLog&) > msgCallbackFcn;
	typedef std::function<void() > timeoutHandlerFcn;

public:
    /**
     * @brief pointer type to the serial port.
     */
    typedef std::shared_ptr< SerialPort > ptr;

    /**
     * @brief Private constructor, only use create method.
     * @param io_service An io_service instance.
     */
    SerialPort( boost::asio::io_service &io_service, sendFunction_t logSendFcn);
    
    /**
     * @brief Destructor.
     */
    virtual ~SerialPort();

    /**
     * @brief Use create to create an instance of the class.
     * @param io_service An io_service instance.
     * @return ptr to the created serial port interface.
     */
    static ptr create( boost::asio::io_service &io_service, sendFunction_t logSendFcn );


    /**
     * @brief start an async read operation on the configured
     * and opened serial port.
     */
    void read( void );

    /**
     * @brief Inquire the status of the port. True means it is ready for a new transmission,
     * false that a tranmission is ongoing.
     **/
    bool ready( void );

    /**
     * @brief Start an async write operation on the configured
     * and opened serial port.
     * @param msg to a QSP instance to be transmitted.
     */
    QGS_ModuleMsgBase::ptr write( QGS_ModuleMsgBase::ptr msg, AppLog &logger);


    /**
     * @brief Close the serial port.
     */
    void close( void );

    /**
     * @brief open the serial port with name port_name.
     * @param port_name Name of the serial port, for example
     *                  "/dev/ttyusb1"
     * @return true if success.
     */
    std::string open( std::string port_name );

    /**
     * Is the serial port already open?
     * @return true if open, false otherwise.
     */
    bool isOpen();

    /**
     * @brief Set the options of the serial port.
     * @param port_name
     * @param baud_rate
     * @param flow_control
     * @param parity
     * @param stop_bits
     */

    void setBaudRate( unsigned int baud_rate );

    void setFlowControl(  b_a_sp::flow_control::type flow_control );

    void setParity( b_a_sp::parity::type parity );
    
    void setStopBits( b_a_sp::stop_bits::type stop_bits );

    /**
     * Set the function to be called when a message is ready.
     * @param fcn
     */
    void setReadCallback( msgCallbackFcn fcn  );

    /**
     * @brief Set the callback function for a successful write operation.
     * 
     * @param fcn 
     */
    void setWriteCallback(  timeoutHandlerFcn fcn  );

    /**
     * Set the function to be called when a read has timed out.
     * @param fcn
     */
    void setReadTimeoutCallback(  timeoutHandlerFcn fcn  );

    void setLogLevel(log_level lvl);

private:


    /**
     * @brief Do Close the serial port.
     * @param error Evaluate this to determine if the
     * operation was successful.
     */
    void doClose( const boost::system::error_code& error );

    /**
     * @brief Callback of the async read operation. If a
     * message handler was registered it will be called here.
     * @param error
     * @param bytes_transferred
     */
    void readCallback( const boost::system::error_code& error,
            std::size_t bytes_transferred );

    /**
     * @brief Callback of the async write operation.
     * @param error
     * @param bytes_transferred
     */
    void writeCallback( const boost::system::error_code& error,
            std::size_t bytes_transferred );

    /**
     * @brief write timeout timer callback.
     * @param error
     */
    void timerWriteCallback( const boost::system::error_code& error );


    std::string mName;
    unsigned int mBaudRate;
    enum b_a_sp::flow_control::type mFlowControl;
    enum b_a_sp::parity::type mParity;
    enum b_a_sp::stop_bits::type mStopBits;
    boost::asio::io_service& mIoService;
    boost::asio::serial_port mSerialPort;
    boost::system::error_code mError;
    boost::asio::deadline_timer mTimeoutWrite;
    boost::asio::streambuf mStreambuf;
    bool mWriteOngoing;
    std::string mReceivedData;
    Parser mParser;
    msgCallbackFcn mMessageHandler;
    timeoutHandlerFcn mReadTimeoutHandler;
    timeoutHandlerFcn mWriteCallback;
    AppLog log;
    AppLog asioLog;

};

} /* namespace QuadGS */


#endif /* SERIALPORT_H_ */
