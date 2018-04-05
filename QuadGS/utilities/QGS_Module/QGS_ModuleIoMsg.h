/*
 * QGS_ModuleIoMsg.h
 *
 *  Created on: Mar 27, 2018
 *      Author: mlundh
 */

#ifndef QUADGS_UTILITIES_IOMESSAGES_QGS_MODULEIOMSG_H_
#define QUADGS_UTILITIES_IOMESSAGES_QGS_MODULEIOMSG_H_
#include "QGS_ModuleMsg.h"
namespace QuadGS {

template<class IoMessage>
class QGS_ModuleIoMsg: public QGS_ModuleMsg<QGS_ModuleIoMsg<IoMessage>>
{
public:

	QGS_ModuleIoMsg(std::unique_ptr<IoMessage> msg, messageTypes_t type)
	:QGS_ModuleMsg<QGS_ModuleIoMsg<IoMessage>>(type),mIoMsg(std::move(msg))
	{

	}

	QGS_ModuleIoMsg(const QGS_ModuleIoMsg<IoMessage>& msg)
	:QGS_ModuleMsg<QGS_ModuleIoMsg<IoMessage>>(msg), mIoMsg(std::move(msg.getIoMsgClone()))
	{

	}

	virtual ~QGS_ModuleIoMsg()
	{

	}

	DISPATCH_FCN

	typedef std::unique_ptr<QGS_ModuleIoMsg<IoMessage>> ptr;

	std::unique_ptr<IoMessage> getIoMsg()
	{
		return std::move(mIoMsg);
	}

	std::unique_ptr<IoMessage> getIoMsgClone() const
	{
		return std::make_unique<IoMessage>(*mIoMsg);
	}

	void setIoMsg(std::unique_ptr<IoMessage> msg)
	{
		mIoMsg = std::move(msg);
	}

	virtual BinaryOStream& stream(BinaryOStream& os) const
	{
		QGS_ModuleMsgBase::stream(os);
		os << *mIoMsg;
		return os;
	}
	virtual BinaryIStream& stream(BinaryIStream& is)
	{
		QGS_ModuleMsgBase::stream(is);
		is >> *mIoMsg;
		return is;
	}

private:
	std::unique_ptr<IoMessage> mIoMsg;
};
}



#endif /* QUADGS_UTILITIES_IOMESSAGES_QGS_MODULEIOMSG_H_ */
