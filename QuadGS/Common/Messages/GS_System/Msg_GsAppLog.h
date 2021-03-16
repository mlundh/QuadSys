#ifndef QUADGS_MESSAGE_MSG_GSAPPLOG_H_
#define QUADGS_MESSAGE_MSG_GSAPPLOG_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_GsAppLog: public QGS_ModuleMsg<Msg_GsAppLog>
{
public:

	Msg_GsAppLog(QGS_ModuleMsgBase& msg);

	Msg_GsAppLog(const msgAddr_t destination, std::string message);

	Msg_GsAppLog(const Msg_GsAppLog& msg);

	virtual ~Msg_GsAppLog();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_GsAppLog> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::string getMessage() const;

	void setMessage(std::string message);
	

	
private:
	std::string mMessage;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_GSAPPLOG_H_ */

