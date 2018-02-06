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

#include "AppLog.h"

#include "BinaryStream.h"


#define DISPATCH_FCN virtual void dispatch(QGS_MessageHandlerBase* handler) { dynamicDispatch(handler,this); }

namespace QuadGS {

enum messageTypes
{
	msgFc,
	msgParam,
	msgLog,
	msgDebug,
	msgCommand,
	msgSubscription,
	msgQuit,
};
typedef enum messageTypes messageTypes_t; // TODO remove this. Should be derived classes instead.

class QGS_MessageHandlerBase
{
public:
	QGS_MessageHandlerBase(std::string name):mLogger(name)
	{

	}
	virtual ~QGS_MessageHandlerBase() = default;
	virtual void unhandledMsg()
	{
		mLogger.QuadLog( severity_level::warning ,"Received unhandled message. Install handler or remove subscription.");
	}
protected:
	AppLog mLogger;
	std::vector<messageTypes_t> subscriptions;
};

class QGS_Msg;

template<class T, class B>
struct Derived_from
{
	static void constraints(T* p)
	{
		B* pb = p; // will fail compilation if pb is not a base of p
		(void)pb;
	}
	Derived_from()
	{
		void(*p)(T*) = constraints; //Function pointer to the constraints function.
		(void)p;
	}
};

template<typename MessageType>
class QGS_MessageHandler: public virtual QGS_MessageHandlerBase
{
public:
	QGS_MessageHandler():QGS_MessageHandlerBase("baseHandler")
{
		Derived_from<MessageType, QGS_Msg>();
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
	friend BinaryOStream& operator<< (BinaryOStream& os, const QGS_Msg& pl);
	friend BinaryIStream& operator>> (BinaryIStream& is, QGS_Msg& pl);


	QGS_Msg()
	{

	}
protected:
	template<typename MessageType>
	void dynamicDispatch(QGS_MessageHandlerBase* handler,MessageType* self)
	{
		Derived_from<MessageType, QGS_Msg>(); // Compile time type check.
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
	 * Dispatch the processing. The processing will be handled by the appropriate message handler. The
	 * correct handler for this type will be automatically called. Implementation should look like this:
	 *
	 *  void dispatch(QGS_MessageHandlerBase* handler)
	 *  {
	 *      dynamicDispatch(handler,this);
	 *  }
	 *
	 *  anything else is an error. You can use the macro DISPATCH_FCN(handler) as a convenience.
	 *
	 * @param handler
	 */
	virtual void dispatch(QGS_MessageHandlerBase* handler) {};
	/**
	 * Shared pointer type for this type.
	 */
	typedef std::shared_ptr<QGS_Msg> Ptr;

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
