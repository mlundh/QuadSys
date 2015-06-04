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
class transfer_until
{
public:
  typedef std::size_t result_type;

  explicit transfer_until(uint8_t delimiter, uint8_t* buffer)
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
class Serial_Port
        : public std::enable_shared_from_this< Serial_Port >
        , public Log
{
public:



    /**
     * @brief pointer type to the serial port.
     */
    typedef std::shared_ptr< Serial_Port > pointer;

public:
    /**
     * @brief Destructor.
     */
    virtual ~Serial_Port();

    /**
     * @brief Use create to create an instance of the class.
     * @param io_service An io_service instance.
     * @return ptr to the created serial port interface.
     */
    static pointer create( boost::asio::io_service &io_service );

    /**
     * @brief start an async read operation on the configured
     * and opened serial port.
     */
    void do_read( void );

    /**
     * @brief Start an async write operation on the configured
     * and opened serial port.
     * @param buffer Buffer containing the data to be transmitted.
     * @param buffer_length Length of data to be transmitted.
     */
    void do_write( unsigned char *buffer, uint8_t buffer_length );

    /**
     * @brief Start an async write operation on the configured
     * and opened serial port.
     * @param ptr to a QSP instance to be transmitted.
     */
    void do_write( QspPayloadRaw::Ptr ptr);

   void start_read_timer(int timeout = 1000);

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
    void set_name( std::string port_name );

    void set_baud_rate( unsigned int baud_rate );

    void set_flow_control(  b_a_sp::flow_control::type flow_control );

    void set_parity( b_a_sp::parity::type parity );
    
    void set_stop_bits( b_a_sp::stop_bits::type stop_bits );
    
    void set_read_callback(  IoBase::MessageHandlerFcn fcn  );

private:
    /**
     * @brief Private constructor, only use create method.
     * @param io_service An io_service instance.
     */
    Serial_Port( boost::asio::io_service &io_service );

    /**
     * @brief Do Close the serial port.
     * @param error Evaluate this to determine if the
     * operation was successful.
     */
    void do_close( const boost::system::error_code& error );

    /**
     * @brief Callback of the async read operation. If a
     * message handler was registered it will be called here.
     * @param error
     * @param bytes_transferred
     */
    void read_callback( const boost::system::error_code& error,
            std::size_t bytes_transferred );

    /**
     * @brief Callback of the async write operation.
     * @param error
     * @param bytes_transferred
     */
    void write_callback( const boost::system::error_code& error,
            std::size_t bytes_transferred );

    /**
     * @brief Read timeout timer callback.
     * @param error
     */
    void timer_read_callback( const boost::system::error_code& error );

    /**
     * @brief Write timeout timer callback.
     * @param error
     */
    void timer_write_callback( const boost::system::error_code& error );


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
