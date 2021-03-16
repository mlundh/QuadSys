#ifndef QUADGS_MESSAGE_MSG_APPLOG_H_
#define QUADGS_MESSAGE_MSG_APPLOG_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_AppLog: public QGS_ModuleMsg<Msg_AppLog>
{
public:

	Msg_AppLog(QGS_ModuleMsgBase& msg);

	Msg_AppLog(const msgAddr_t destination, std::string message);

	Msg_AppLog(const Msg_AppLog& msg);

	virtual ~Msg_AppLog();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_AppLog> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::string getMessage() const;

	void setMessage(std::string message);
	

	
private:
	std::string mMessage;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_APPLOG_H_ */

