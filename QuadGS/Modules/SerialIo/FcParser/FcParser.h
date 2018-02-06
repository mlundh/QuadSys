/*
 * FcParser.h
 *
 *  Copyright (C) 2017 Martin Lundh
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

#ifndef IO_SERIAL_PARSER_SRC_FCPARSER_H_
#define IO_SERIAL_PARSER_SRC_FCPARSER_H_
#include "QGS_IoInterface.h"

namespace QuadGS {

	class FcParser: public ParserBase
	{
	public:
		FcParser();
		virtual ~FcParser();

		/**
		 * Parse the data in the given vector.
		 * Return: 0 if everything is ok.
		 * 		   negative if there is not enough data in the vector.
		 * 		   positive if there is still data in the vector.
		 * @param data	buffer of raw data.
		 * @return	according to above.
		 */
		virtual int parse( std::shared_ptr<std::vector<unsigned char> > data);

		/**
		 * Get the shared ptr of the header. Will return a NULL ptr if no data has
		 * been parsed, or if there was no header data. After the function is called
		 * the internal shared ptr is reset so that another call to getHeader will
		 * return a null ptr.
		 * @return pointer to a header.
		 */
		virtual std::shared_ptr<QGS_MsgHeader> getHeader( void );

		/**
		 * Get the shared ptr of the payload. Will return a NULL ptr if no data has
		 * been parsed, or if there was no payload data. After the function is called
		 * the internal shared ptr is reset so that another call to getPayload will
		 * return a null ptr.
		 * @return
		 */
		virtual std::shared_ptr<QGS_Msg> getPayload( void );
	};

} /* namespace QuadGS */

#endif /* IO_SERIAL_PARSER_SRC_FCPARSER_H_ */
