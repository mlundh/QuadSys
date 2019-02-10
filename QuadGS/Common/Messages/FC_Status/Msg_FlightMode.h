#ifndef QUADGS_MESSAGE_MSG_FLIGHTMODE_H_
#define QUADGS_MESSAGE_MSG_FLIGHTMODE_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_FlightMode: public QGS_ModuleMsg<Msg_FlightMode>
{
public:

	Msg_FlightMode(QGS_ModuleMsgBase& msg);

	Msg_FlightMode(const msgAddr_t destination, uint8_t mode);

	Msg_FlightMode(const Msg_FlightMode& msg);

	virtual ~Msg_FlightMode();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_FlightMode> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint8_t getMode() const;

	void setMode(uint8_t mode);
	

	
private:
	uint8_t mMode;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_FLIGHTMODE_H_ */

