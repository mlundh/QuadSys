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
#include <iostream>
#include <sstream>

#include "messageTypes.h"
#include "AppLog.h"
#include "BinaryStream.h"
#include "messageTypes.h"

#define DISPATCH_FCN virtual void dispatch(QGS_MessageHandlerBase* handler) { this->dynamicDispatch(handler,this); }

namespace QuadGS {

class QGS_MessageHandlerBase
{
public:
	QGS_MessageHandlerBase(std::string name):mLogger(name), mName(name)
	{

	}
	virtual ~QGS_MessageHandlerBase() = default;

	virtual void unhandledMsg()
	{
		mLogger.QuadLog( severity_level::error ,"Received unhandled message.");
	}

	std::string getName()
	{
		return mName;
	}

protected:
	AppLog mLogger;
	std::string mName;
};


class CloneBase
{
public:
	virtual ~CloneBase(){};
    virtual CloneBase* clone() const = 0;
};

/**
 *
 */
template <class MessageType>
class Cloneable : virtual public CloneBase
{
public:
	virtual ~Cloneable(){};
	virtual MessageType* clone() const
	{
		MessageType *msg = new MessageType(*this);
		return msg;
	}
};



class QGS_Msg;

template <class MessageType,
class B = QGS_Msg, class = typename std::enable_if<std::is_base_of<B,MessageType>::value>::type>
class QGS_MessageHandler: public virtual QGS_MessageHandlerBase
{
public:
	QGS_MessageHandler():QGS_MessageHandlerBase("baseHandler")
{
}
	virtual ~QGS_MessageHandler() = default;
	virtual void process(MessageType*)=0;

};



/**
 * Base class of all message types in QuadGS. Inherit from this when creating new message
 * types to be able to pass by base.
 */
class QGS_Msg
{
public:

	QGS_Msg()
	{

	}
protected:
	template <class MessageType,
	class B = QGS_Msg, class = typename std::enable_if<std::is_base_of<B,MessageType>::value>::type>
	void dynamicDispatch(QGS_MessageHandlerBase* handler,MessageType* self)
	{
		QGS_MessageHandler<MessageType>* msgHandler = dynamic_cast<QGS_MessageHandler<MessageType>*>(handler);
		if(msgHandler != nullptr)
		{
			msgHandler->process(self);
		}
		else
		{
			handler->unhandledMsg();
		}
	}

public:

	/**
	 * Shared pointer type for this type.
	 */
	typedef std::shared_ptr<QGS_Msg> Ptr;

	/**
	 * Dispatch the processing. The processing will be handled by the appropriate message handler. The
	 * correct handler for this type will be automatically called. Implementation should look like this:
	 *
	 *  void dispatch(QGS_MessageHandlerBase* handler)
	 *  {
	 *      dynamicDispatch(handler,this);
	 *  }
	 *
	 *  anything else is an error. You can use the macro DISPATCH_FCN as a convenience.
	 *
	 * @param handler
	 */
	virtual void dispatch(QGS_MessageHandlerBase* handler) {};


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

template <class T,
class B = QGS_Msg, class = typename std::enable_if<std::is_base_of<B,T>::value>::type>
BinaryOStream& operator <<(BinaryOStream& os, const T& pl)
{
	pl.stream(os);
	return os;
}

template <class T,
class B = QGS_Msg, class = typename std::enable_if<std::is_base_of<B,T>::value>::type>
BinaryIStream& operator >>(BinaryIStream& is, T& pl)
{
	pl.stream(is);
	return is;
}

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_SERIAL_MANAGER_SRC_QSPPAYLOADBASE_H_ */
