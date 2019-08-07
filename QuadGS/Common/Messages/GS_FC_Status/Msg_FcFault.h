#ifndef QUADGS_MESSAGE_MSG_FCFAULT_H_
#define QUADGS_MESSAGE_MSG_FCFAULT_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_FcFault: public QGS_ModuleMsg<Msg_FcFault>
{
public:

	Msg_FcFault(QGS_ModuleMsgBase& msg);

	Msg_FcFault(const msgAddr_t destination);

	Msg_FcFault(const Msg_FcFault& msg);

	virtual ~Msg_FcFault();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_FcFault> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	

	
private:
	
};
}

#endif /* QUADGS_MESSAGE_MSG_FCFAULT_H_ */

