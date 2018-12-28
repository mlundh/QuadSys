#ifndef QUADGS_MESSAGE_MSG_IOWRAPPER_H_
#define QUADGS_MESSAGE_MSG_IOWRAPPER_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_IoWrapper: public QGS_ModuleMsg<Msg_IoWrapper>
{
public:

	Msg_IoWrapper(QGS_ModuleMsgBase& msg);

	Msg_IoWrapper(const msgAddr_t destination, std::unique_ptr<QGS_ModuleMsgBase> baseMsg);

	Msg_IoWrapper(const Msg_IoWrapper& msg);

	virtual ~Msg_IoWrapper();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_IoWrapper> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::unique_ptr<QGS_ModuleMsgBase> getCloneBasemsg() const;

	std::unique_ptr<QGS_ModuleMsgBase> getBasemsg() ;

	void setBasemsg(std::unique_ptr<QGS_ModuleMsgBase> baseMsg);

	
private:
	std::unique_ptr<QGS_ModuleMsgBase> mBasemsg;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_IOWRAPPER_H_ */

