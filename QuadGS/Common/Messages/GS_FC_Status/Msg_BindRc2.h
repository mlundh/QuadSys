#ifndef QUADGS_MESSAGE_MSG_BINDRC2_H_
#define QUADGS_MESSAGE_MSG_BINDRC2_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_BindRc2: public QGS_ModuleMsg<Msg_BindRc2>
{
public:

	Msg_BindRc2(QGS_ModuleMsgBase& msg);

	Msg_BindRc2(const msgAddr_t destination);

	Msg_BindRc2(const Msg_BindRc2& msg);

	virtual ~Msg_BindRc2();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_BindRc2> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	

	
private:
	
};
}

#endif /* QUADGS_MESSAGE_MSG_BINDRC2_H_ */

