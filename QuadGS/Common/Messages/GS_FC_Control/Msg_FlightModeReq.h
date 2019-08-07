#ifndef QUADGS_MESSAGE_MSG_FLIGHTMODEREQ_H_
#define QUADGS_MESSAGE_MSG_FLIGHTMODEREQ_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_FlightModeReq: public QGS_ModuleMsg<Msg_FlightModeReq>
{
public:

	Msg_FlightModeReq(QGS_ModuleMsgBase& msg);

	Msg_FlightModeReq(const msgAddr_t destination, uint8_t mode);

	Msg_FlightModeReq(const Msg_FlightModeReq& msg);

	virtual ~Msg_FlightModeReq();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_FlightModeReq> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint8_t getMode() const;

	void setMode(uint8_t mode);
	

	
private:
	uint8_t mMode;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_FLIGHTMODEREQ_H_ */

