#ifndef QUADGS_MESSAGE_MSG_CTRLMODE_H_
#define QUADGS_MESSAGE_MSG_CTRLMODE_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_CtrlMode: public QGS_ModuleMsg<Msg_CtrlMode>
{
public:

	Msg_CtrlMode(QGS_ModuleMsgBase& msg);

	Msg_CtrlMode(const msgAddr_t destination, uint8_t mode);

	Msg_CtrlMode(const Msg_CtrlMode& msg);

	virtual ~Msg_CtrlMode();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_CtrlMode> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint8_t getMode() const;

	void setMode(uint8_t mode);
	

	
private:
	uint8_t mMode;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_CTRLMODE_H_ */

