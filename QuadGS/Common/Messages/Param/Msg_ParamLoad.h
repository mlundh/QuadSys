#ifndef QUADGS_MESSAGE_MSG_PARAMLOAD_H_
#define QUADGS_MESSAGE_MSG_PARAMLOAD_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_ParamLoad: public QGS_ModuleMsg<Msg_ParamLoad>
{
public:

	Msg_ParamLoad(QGS_ModuleMsgBase& msg);

	Msg_ParamLoad(const msgAddr_t destination);

	Msg_ParamLoad(const Msg_ParamLoad& msg);

	virtual ~Msg_ParamLoad();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_ParamLoad> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	

	
private:
	
};
}

#endif /* QUADGS_MESSAGE_MSG_PARAMLOAD_H_ */

