#ifndef QUADGS_MESSAGE_MSG_BINDRC1_H_
#define QUADGS_MESSAGE_MSG_BINDRC1_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_BindRc1: public QGS_ModuleMsg<Msg_BindRc1>
{
public:

	Msg_BindRc1(QGS_ModuleMsgBase& msg);

	Msg_BindRc1(const msgAddr_t destination);

	Msg_BindRc1(const Msg_BindRc1& msg);

	virtual ~Msg_BindRc1();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_BindRc1> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	

	
private:
	
};
}

#endif /* QUADGS_MESSAGE_MSG_BINDRC1_H_ */

