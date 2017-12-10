/*
 * IoInterface.h
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

#ifndef QUADGS_MODULES_USERINTERFACE_IOBASE_H_
#define QUADGS_MODULES_USERINTERFACE_IOBASE_H_
#include <memory>
#include <functional>
#include <vector>
namespace QuadGS {

class QGS_CoreInterface;
class QGS_UiCommand;
class QGS_MsgHeader;
class QGS_Msg;

class QGS_IoInterface
{
public:
	/**
	 * @brief Callback typedefs.
	 */
	typedef std::function<void( std::shared_ptr<QGS_MsgHeader>, std::shared_ptr<QGS_Msg> )> MessageHandlerFcn;
	typedef std::function<void( void )> TimeoutHandlerFcn;

	QGS_IoInterface(){}
	virtual ~QGS_IoInterface(){}

	/**
	 * Write a message consisting of a header and payload to the io module.
	 * @param header	Header data
	 * @param data		Payload data.
	 */
	virtual void write( std::shared_ptr<QGS_MsgHeader> header, std::shared_ptr<QGS_Msg> payload) = 0;

	/**
	 * Start a read operation. The read operation will continue until the program
	 * is closed, or an error occurs.
	 */
	virtual void startRead( void ) = 0;

	/**
	 * Set the function to be called when a new message has arrived.
	 * @param The fuction to be called.
	 */
	virtual void setMessageCallback( MessageHandlerFcn ) = 0;

	/**
	 * Get the available commands of the io module. These are for human interaction
	 * usage only.
	 * @return A vector with pointer to commands.
	 */
	virtual std::vector< std::shared_ptr<QGS_UiCommand> > getCommands( ) = 0;


	virtual std::string getStatus( ) = 0;
};

/**
 * @class ParserBase
 * Abstract base to all parsers that the i/o module should use.
 */
class ParserBase
{
public:
	/**
	 * The parser should always be passed by shared ptr.
	 */
	typedef std::shared_ptr<ParserBase> ptr;

	/**
	 * Default constructor
	 */
	ParserBase(){}

	/**
	 * Virtual destructor
	 */
	virtual ~ParserBase(){}

	/**
	 * Parse the data in the given vector.
	 * Return: 0 if everything is ok.
	 * 		   negative if there is not enough data in the vector.
	 * 		   positive if there is still data in the vector.
	 * @param data	buffer of raw data.
	 * @return	according to above.
	 */
	virtual int parse( std::shared_ptr<std::vector<unsigned char> > data) = 0;

	/**
	 * Get the shared ptr of the header. Will return a NULL ptr if no data has
	 * been parsed, or if there was no header data. After the function is called
	 * the internal shared ptr is reset so that another call to getHeader will
	 * return a null ptr.
	 * @return pointer to a header.
	 */
	virtual std::shared_ptr<QGS_MsgHeader> getHeader( void ) = 0;

	/**
	 * Get the shared ptr of the payload. Will return a NULL ptr if no data has
	 * been parsed, or if there was no payload data. After the function is called
	 * the internal shared ptr is reset so that another call to getPayload will
	 * return a null ptr.
	 * @return
	 */
	virtual std::shared_ptr<QGS_Msg> getPayload( void ) = 0;

protected:
	std::shared_ptr<QGS_MsgHeader> mHeader;
	std::shared_ptr<QGS_Msg> mPayload;
};


} /* namespace QuadGS */

#endif /* QUADGS_MODULES_USERINTERFACE_IOBASE_H_ */
