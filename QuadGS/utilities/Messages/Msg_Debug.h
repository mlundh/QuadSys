#ifndef QUADGS_MESSAGE_MSG_DEBUG_H_
#define QUADGS_MESSAGE_MSG_DEBUG_H_
#include "QGS_ModuleMsg.h"
#include "QGS_DebugMsg.h"

namespace QuadGS {

class Msg_Debug: public QGS_ModuleMsg<Msg_Debug>
{
public:

	Msg_Debug(std::string destination, std::unique_ptr<QGS_DebugMsg> dbgMsg);

	Msg_Debug(const Msg_Debug& msg);

	virtual ~Msg_Debug();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Debug> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::unique_ptr<QGS_DebugMsg> getCloneDbgmsg() const;

	std::unique_ptr<QGS_DebugMsg> getDbgmsg() ;

	void setDbgmsg(std::unique_ptr<QGS_DebugMsg> dbgMsg);

	
private:
	std::unique_ptr<QGS_DebugMsg> mDbgmsg;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_DEBUG_H_ */

