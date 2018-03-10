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
#include "QGS_Msg.h"
#include "QGS_IoInterface.h"

typedef boost::asio::serial_port b_a_sp;

namespace QuadGS {

/**
 * @brief Helper class to boost async_read_untill
 */
class transferUntil
{
public:
  typedef std::size_t result_type;

  explicit transferUntil(uint8_t delimiter, std::vector<unsigned char>& buffer)
    : mDelimiter(delimiter)
    , mBuffer(buffer)
  {
  }

  template <typename Error>
  std::size_t operator()(const Error& err, std::size_t bytes_transferred)
  {
      if((bytes_transferred >= 4) && (mBuffer[ bytes_transferred - 1 ] == mDelimiter) && (!err))
      {
          return 0;
      }
      return 1;
  }

private:
  uint8_t mDelimiter;
  std::vector<unsigned char>& mBuffer;
};

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
        , public AppLog
{
public:
    /**
     * @brief pointer type to the serial port.
     */
    typedef std::shared_ptr< SerialPort > ptr;

    /**
     * @brief Destructor.
     */
    virtual ~SerialPort();

    /**
     * @brief Use create to create an instance of the class.
     * @param io_service An io_service instance.
     * @return ptr to the created serial port interface.
     */
    static ptr create( boost::asio::io_service &io_service );

    /**
     * @brief start an async read operation on the configured
     * and opened serial port.
     */
    void read( void );

    /**
     * @brief Start an async write operation on the configured
     * and opened serial port.
     * @param ptr to a QSP instance to be transmitted.
     */
    void write( std::shared_ptr<QGS_IoHeader> header, std::shared_ptr<QGS_Msg> payload);

    /**
     * Start the read timer. If timeout occurs then the read will be considered failed.
     * @param timeout
     */
   void startReadTimer(int timeout = 1000);

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
    void open( std::string port_name );

    /**
     * @brief Set the options of the serial port.
     * @param port_name
     * @param baud_rate
     * @param flow_control
     * @param parity
     * @param stop_bits
     */
    void setName( std::string port_name );

    void setBaudRate( unsigned int baud_rate );

    void setFlowControl(  b_a_sp::flow_control::type flow_control );

    void setParity( b_a_sp::parity::type parity );
    
    void setStopBits( b_a_sp::stop_bits::type stop_bits );
    
    /**
     * Set the parser that parses raw data into messages.
     * @param fcn
     */
    void setParser(  ParserBase::ptr parser  );

    /**
     * Set the function to be called when a message is ready.
     * @param fcn
     */
    void setReadCallback( QGS_IoInterface::MessageHandlerFcn fcn  );

    /**
     * Set the function to be called when a read has timed out.
     * @param fcn
     */
    void setReadTimeoutCallback(  QGS_IoInterface::TimeoutHandlerFcn fcn  );

private:
    /**
     * @brief Private constructor, only use create method.
     * @param io_service An io_service instance.
     */
    SerialPort( boost::asio::io_service &io_service );

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
     * @brief read timeout timer callback.
     * @param error
     */
    void timerReadCallback( const boost::system::error_code& error );

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
    boost::asio::deadline_timer mTimeoutRead;
    boost::asio::deadline_timer mTimeoutWrite;
    std::shared_ptr<std::vector<unsigned char> > mWriteBuff;
    std::shared_ptr<std::vector<unsigned char> > mReadBuff;
    ParserBase::ptr mParser;
    QGS_IoInterface::MessageHandlerFcn mMessageHandler;
    QGS_IoInterface::TimeoutHandlerFcn mReadTimeoutHandler;
};

} /* namespace QuadGS */


#endif /* SERIALPORT_H_ */
