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
#include "msgAddr.h"
#include "common_types.h"

namespace QuadGS {
class QGS_ModuleMsgBase;
typedef std::function<void(std::unique_ptr<QGS_ModuleMsgBase>) > WriteFcn;


class QGS_ModuleMsgBase: public QGS_Msg
{
public:

	typedef std::unique_ptr<QGS_ModuleMsgBase> ptr;

	QGS_ModuleMsgBase();

	QGS_ModuleMsgBase(messageTypes_t type, const msgAddr_t desination);

	QGS_ModuleMsgBase(const QGS_ModuleMsgBase& msg);

	DISPATCH_FCN

	virtual ~QGS_ModuleMsgBase();

	void setSource(msgAddr_t originator);

	msgAddr_t getSource() const;

	void setDestination(msgAddr_t port);

	msgAddr_t getDestination() const;

	messageTypes_t getType() const;

	void setMsgNr(uint8_t nr);

	uint8_t getMsgNr() const;

	void setSkipStreamHeader();

	std::string toString() const;

	virtual BinaryOStream& stream(BinaryOStream& os) const;

	virtual BinaryIStream& stream(BinaryIStream& is);

	virtual QGS_ModuleMsgBase* clone() const {return new QGS_ModuleMsgBase(*this);};

private:
	uint32_t mType;
	uint32_t mSource;
	uint32_t mDestination;
	std::uint8_t mMsgNr;
	bool mSkipStreamHeader = false;
};


template <class MessageType>
class QGS_ModuleMsg : public QGS_ModuleMsgBase
{
public:

	QGS_ModuleMsg(messageTypes_t type, const msgAddr_t desination)
	:QGS_ModuleMsgBase(type, desination)
	{

	}

	QGS_ModuleMsg(const QGS_ModuleMsgBase& msg)
	:QGS_ModuleMsgBase(msg)
	{

	}
	QGS_ModuleMsg(const QGS_ModuleMsg& msg)
	:QGS_ModuleMsgBase(msg)
	{

	}

	virtual ~QGS_ModuleMsg(){};
	virtual QGS_ModuleMsg* clone() const
	{
		return new MessageType(static_cast<const MessageType&>(*this));
	}
};

} /* namespace QuadGS */

#endif /* QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_MODULEMSG_H_ */
