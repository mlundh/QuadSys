#ifndef QUADGS_MESSAGE_MSG_GSHASLOG_H_
#define QUADGS_MESSAGE_MSG_GSHASLOG_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_GsHasLog: public QGS_ModuleMsg<Msg_GsHasLog>
{
public:

	Msg_GsHasLog(QGS_ModuleMsgBase& msg);

	Msg_GsHasLog(const msgAddr_t destination);

	Msg_GsHasLog(const Msg_GsHasLog& msg);

	virtual ~Msg_GsHasLog();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_GsHasLog> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	

	
private:
	
};
}

#endif /* QUADGS_MESSAGE_MSG_GSHASLOG_H_ */

