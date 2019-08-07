#ifndef QUADGS_MESSAGE_MSG_CTRLMODEREQ_H_
#define QUADGS_MESSAGE_MSG_CTRLMODEREQ_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_CtrlModeReq: public QGS_ModuleMsg<Msg_CtrlModeReq>
{
public:

	Msg_CtrlModeReq(QGS_ModuleMsgBase& msg);

	Msg_CtrlModeReq(const msgAddr_t destination, uint8_t mode);

	Msg_CtrlModeReq(const Msg_CtrlModeReq& msg);

	virtual ~Msg_CtrlModeReq();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_CtrlModeReq> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint8_t getMode() const;

	void setMode(uint8_t mode);
	

	
private:
	uint8_t mMode;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_CTRLMODEREQ_H_ */

