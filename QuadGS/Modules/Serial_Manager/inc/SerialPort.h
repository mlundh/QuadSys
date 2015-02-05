/*
 * SerialPort.h
 *
 *  Created on: Jan 25, 2015
 *      Author: martin
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <Log.h>

typedef boost::asio::serial_port b_a_sp;

namespace QuadGS {

class Serial_Port
        : public boost::enable_shared_from_this< Serial_Port >
        , public Log
{
public:

    typedef boost::shared_ptr< Serial_Port > pointer;

private:
    Serial_Port( boost::asio::io_service &io_service );
    public:
    virtual ~Serial_Port();

    static pointer create( boost::asio::io_service &io_service );
    void do_read( void );
    void do_write( unsigned char *buffer );
    void close( void );
    void open( std::string port_name );
    void do_close( const boost::system::error_code& error );
    void set_optins( std::string port_name
            , unsigned int baud_rate
            , b_a_sp::flow_control::type flow_control
            , b_a_sp::parity::type parity
            , b_a_sp::stop_bits::type stop_bits );

private:

    void read_callback( const boost::system::error_code& error,
            std::size_t bytes_transferred );
    void write_callback( const boost::system::error_code& error,
            std::size_t bytes_transferred );
    void timer_read_callback( const boost::system::error_code& error );
    void timer_write_callback( const boost::system::error_code& error );

    std::string mName;
    const unsigned int mBaudRate;
    const enum b_a_sp::flow_control::type mFlowControl;
    const enum b_a_sp::parity::type mParity;
    const enum b_a_sp::stop_bits::type mStopBits;
    boost::asio::io_service& mIoService;
    boost::asio::serial_port mSerialPort;
    boost::system::error_code mError;
    unsigned char mBufferRead[ 6 ];
    unsigned char mBufferWrite[ 6 ];
    boost::asio::deadline_timer mTimeoutRead;
    boost::asio::deadline_timer mTimeoutWrite;
};

} /* namespace QuadGS */

#endif /* SERIALPORT_H_ */
