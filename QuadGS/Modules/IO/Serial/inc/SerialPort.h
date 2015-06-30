/*
 * SerialPort.h
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <cstddef>
#include <memory>

#include "QspPayloadRaw.h"
#include "Log.h"
#include "IoBase.h"

typedef boost::asio::serial_port b_a_sp;

namespace QuadGS {
/**
 * @brief Helper class to boost async_read_untill
 */
class transferUntil
{
public:
  typedef std::size_t result_type;

  explicit transferUntil(uint8_t delimiter, uint8_t* buffer)
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
  uint8_t* mBuffer;
};

/**
 * Class containing everything to do with serial communication
 * in this program.
 */
class SerialPort
        : public std::enable_shared_from_this< SerialPort >
        , public Log
{
public:



    /**
     * @brief pointer type to the serial port.
     */
    typedef std::shared_ptr< SerialPort > ptr;

public:
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
    void doRead( void );

    /**
     * @brief Start an async write operation on the configured
     * and opened serial port.
     * @param buffer Buffer containing the data to be transmitted.
     * @param buffer_length Length of data to be transmitted.
     */
    void doWrite( unsigned char *buffer, uint8_t buffer_length );

    /**
     * @brief Start an async write operation on the configured
     * and opened serial port.
     * @param ptr to a QSP instance to be transmitted.
     */
    void doWrite( QspPayloadRaw::Ptr ptr);

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
    
    void setReadCallback(  IoBase::MessageHandlerFcn fcn  );

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
     * @brief Read timeout timer callback.
     * @param error
     */
    void timerReadCallback( const boost::system::error_code& error );

    /**
     * @brief Write timeout timer callback.
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
    QspPayloadRaw::Ptr mPayloadWrite;
    QspPayloadRaw::Ptr mPayloadRead;
    IoBase::MessageHandlerFcn mMessageHandler;
};

} /* namespace QuadGS */


#endif /* SERIALPORT_H_ */
