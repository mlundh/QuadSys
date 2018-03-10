/*
 * QGS_ModuleMsg.h
 *
 *  Copyright (C) 2018 Martin Lundh
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

#ifndef QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_MODULEMSG_H_
#define QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_MODULEMSG_H_
#include "QGS_Msg.h"

namespace QuadGS {
class QGS_ModuleMsg;
typedef std::function<void(std::unique_ptr<QGS_ModuleMsg>) > WriteFcn;


class QGS_ModuleMsg: public QGS_Msg
{
public:

	typedef std::unique_ptr<QGS_ModuleMsg> ptr;

	static ptr Create(QGS_ModuleMsg& copy);

	QGS_ModuleMsg(messageTypes_t type);

	QGS_ModuleMsg(const QGS_ModuleMsg& msg);

	DISPATCH_FCN
	/*virtual void dispatch(QGS_MessageHandlerBase* handler)
	{
		dynamicDispatch(handler,this);
	}*/

	virtual ~QGS_ModuleMsg();

	void setOriginator(std::string originator);

	std::string getOriginator() const;

	void setDestinationPort(int port);

	int getDestinationPort() const;

	void setOriginatingPort(int port);

	int getOriginatingPort() const;

	messageTypes_t getType() const;

	std::string toString() const;

	virtual BinaryOStream& stream(BinaryOStream& os) const;

	virtual BinaryIStream& stream(BinaryIStream& is);

private:
	uint8_t mType;
	int mOriginatingPort;
	int mDestinationPort;
	std::string mOriginator;
};

class QGS_ModuleSubMsg: public QGS_ModuleMsg
{
public:

	QGS_ModuleSubMsg(messageTypes_t type,messageTypes_t subscription);

	virtual ~QGS_ModuleSubMsg();

	DISPATCH_FCN
	typedef std::unique_ptr<QGS_ModuleSubMsg> ptr;

	virtual messageTypes_t getSubscription();
	virtual void setSubscription(messageTypes_t type);

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	virtual BinaryIStream& stream(BinaryIStream& is);

private:
	uint8_t mSubscription;
};


} /* namespace QuadGS */

#endif /* QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_MODULEMSG_H_ */
