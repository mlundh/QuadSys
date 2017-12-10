/*
 * QuadGSMsg.h
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

#ifndef QUADGS_MODULES_SERIAL_MANAGER_SRC_QSPPAYLOADBASE_H_
#define QUADGS_MODULES_SERIAL_MANAGER_SRC_QSPPAYLOADBASE_H_
#include <memory>
#include <string.h>
#include <ostream>

#include "BinaryStream.h"

namespace QuadGS {

/**
 * Base class of all message types in QuadGS. Inherit from this when creating new message
 * types to be able to pass by base.
 */
class QGS_Msg
{
public:
	friend BinaryOStream& operator<< (BinaryOStream& os, const QGS_Msg& pl);
	friend BinaryIStream& operator>> (BinaryIStream& is, QGS_Msg& pl);

	QGS_Msg()
	{

	}
	/**
	 * Shared pointer type for this type.
	 */
	typedef std::shared_ptr<QGS_Msg> QuadGSMsgPtr;

	/**
	 * Get the payload represented as a string.
	 * @return Payload represented as string.
	 */
	virtual std::string toString() const = 0;

	/**
	 * Enable streaming of the class to a BinaryOStream.
	 * @param os
	 * @return
	 */
	virtual BinaryOStream& stream(BinaryOStream& os) const = 0;

	/**
	 * Enable streaming in from a BinaryIStream.
	 * @param is
	 * @return
	 */
	virtual BinaryIStream& stream(BinaryIStream& is) = 0;

	virtual ~QGS_Msg(){};


};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_SERIAL_MANAGER_SRC_QSPPAYLOADBASE_H_ */
