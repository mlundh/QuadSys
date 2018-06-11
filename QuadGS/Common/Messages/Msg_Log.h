#ifndef QUADGS_MESSAGE_MSG_LOG_H_
#define QUADGS_MESSAGE_MSG_LOG_H_
#include "QGS_ModuleMsg.h"
#include "QGS_LogMsg.h"

namespace QuadGS {

class Msg_Log: public QGS_ModuleMsg<Msg_Log>
{
public:

	Msg_Log(std::string destination, std::unique_ptr<QGS_LogMsg> paramMsg);

	Msg_Log(const Msg_Log& msg);

	virtual ~Msg_Log();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Log> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::unique_ptr<QGS_LogMsg> getCloneParammsg() const;

	std::unique_ptr<QGS_LogMsg> getParammsg() ;

	void setParammsg(std::unique_ptr<QGS_LogMsg> paramMsg);

	
private:
	std::unique_ptr<QGS_LogMsg> mParammsg;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_LOG_H_ */

