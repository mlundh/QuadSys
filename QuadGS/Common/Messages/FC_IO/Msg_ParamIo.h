#ifndef QUADGS_MESSAGE_MSG_PARAMIO_H_
#define QUADGS_MESSAGE_MSG_PARAMIO_H_
#include "QGS_ModuleMsg.h"
#include "QGS_ParamMsg.h"

namespace QuadGS {

class Msg_ParamIo: public QGS_ModuleMsg<Msg_ParamIo>
{
public:

	Msg_ParamIo(QGS_ModuleMsgBase& msg);

	Msg_ParamIo(std::string destination, std::unique_ptr<QGSParamMsg> paramMsg);

	Msg_ParamIo(const Msg_ParamIo& msg);

	virtual ~Msg_ParamIo();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_ParamIo> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::unique_ptr<QGSParamMsg> getCloneParammsg() const;

	std::unique_ptr<QGSParamMsg> getParammsg() ;

	void setParammsg(std::unique_ptr<QGSParamMsg> paramMsg);

	
private:
	std::unique_ptr<QGSParamMsg> mParammsg;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_PARAMIO_H_ */

