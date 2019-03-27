#ifndef QUADGS_MESSAGE_MSG_PARAMSAVE_H_
#define QUADGS_MESSAGE_MSG_PARAMSAVE_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_ParamSave: public QGS_ModuleMsg<Msg_ParamSave>
{
public:

	Msg_ParamSave(QGS_ModuleMsgBase& msg);

	Msg_ParamSave(const msgAddr_t destination);

	Msg_ParamSave(const Msg_ParamSave& msg);

	virtual ~Msg_ParamSave();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_ParamSave> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	

	
private:
	
};
}

#endif /* QUADGS_MESSAGE_MSG_PARAMSAVE_H_ */

