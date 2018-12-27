#ifndef QUADGS_MESSAGE_MSG_DEBUGIO_H_
#define QUADGS_MESSAGE_MSG_DEBUGIO_H_
#include "QGS_ModuleMsg.h"
#include "QGS_DebugMsg.h"

namespace QuadGS {

class Msg_DebugIo: public QGS_ModuleMsg<Msg_DebugIo>
{
public:

	Msg_DebugIo(QGS_ModuleMsgBase& msg);

	Msg_DebugIo(std::string destination, std::unique_ptr<QGS_DebugMsg> dbgMsg);

	Msg_DebugIo(const Msg_DebugIo& msg);

	virtual ~Msg_DebugIo();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_DebugIo> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::unique_ptr<QGS_DebugMsg> getCloneDbgmsg() const;

	std::unique_ptr<QGS_DebugMsg> getDbgmsg() ;

	void setDbgmsg(std::unique_ptr<QGS_DebugMsg> dbgMsg);

	
private:
	std::unique_ptr<QGS_DebugMsg> mDbgmsg;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_DEBUGIO_H_ */

