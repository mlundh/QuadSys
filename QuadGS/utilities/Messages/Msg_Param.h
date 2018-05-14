#ifndef QUADGS_MESSAGE_MSG_PARAM_H_
#define QUADGS_MESSAGE_MSG_PARAM_H_
#include "QGS_ModuleMsg.h"
#include "QGS_ParamMsg.h"

namespace QuadGS {

class Msg_Param: public QGS_ModuleMsg<Msg_Param>
{
public:

	Msg_Param(std::string destination, std::unique_ptr<QGSParamMsg> paramMsg);

	Msg_Param(const Msg_Param& msg);

	virtual ~Msg_Param();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Param> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::unique_ptr<QGSParamMsg> getCloneParammsg() const;

	std::unique_ptr<QGSParamMsg> getParammsg() ;

	void setParammsg(std::unique_ptr<QGSParamMsg> paramMsg);

	
private:
	std::unique_ptr<QGSParamMsg> mParammsg;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_PARAM_H_ */

