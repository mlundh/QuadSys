/*
 * Parser.h
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
#include <memory>
#include <vector>
#include "QGS_ModuleMsg.h"

namespace QuadGS {

	class Parser
	{
	public:
		Parser();
		virtual ~Parser();

		/**
		 * Parse the data in the given vector.
		 * @param data	buffer of raw data.
		 * @return	Pointer to the parsed message. Null if failed.
		 */
		QGS_ModuleMsgBase::ptr parse( std::vector<unsigned char> data);
	};

} /* namespace QuadGS */

#endif /* IO_SERIAL_PARSER_SRC_FCPARSER_H_ */
