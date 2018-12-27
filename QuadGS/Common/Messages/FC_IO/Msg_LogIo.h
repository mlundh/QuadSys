#ifndef QUADGS_MESSAGE_MSG_LOGIO_H_
#define QUADGS_MESSAGE_MSG_LOGIO_H_
#include "QGS_ModuleMsg.h"
#include "QGS_LogMsg.h"

namespace QuadGS {

class Msg_LogIo: public QGS_ModuleMsg<Msg_LogIo>
{
public:

	Msg_LogIo(QGS_ModuleMsgBase& msg);

	Msg_LogIo(std::string destination, std::unique_ptr<QGS_LogMsg> paramMsg);

	Msg_LogIo(const Msg_LogIo& msg);

	virtual ~Msg_LogIo();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_LogIo> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::unique_ptr<QGS_LogMsg> getCloneParammsg() const;

	std::unique_ptr<QGS_LogMsg> getParammsg() ;

	void setParammsg(std::unique_ptr<QGS_LogMsg> paramMsg);

	
private:
	std::unique_ptr<QGS_LogMsg> mParammsg;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_LOGIO_H_ */

