#ifndef QUADGS_MESSAGE_MSG_NEWSETPOINT_H_
#define QUADGS_MESSAGE_MSG_NEWSETPOINT_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_NewSetpoint: public QGS_ModuleMsg<Msg_NewSetpoint>
{
public:

	Msg_NewSetpoint(QGS_ModuleMsgBase& msg);

	Msg_NewSetpoint(const msgAddr_t destination, state_data_t setpoint, uint8_t priority, uint8_t validFor);

	Msg_NewSetpoint(const Msg_NewSetpoint& msg);

	virtual ~Msg_NewSetpoint();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_NewSetpoint> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	state_data_t getSetpoint() const;

	void setSetpoint(state_data_t setpoint);
	
	uint8_t getPriority() const;

	void setPriority(uint8_t priority);
	
	uint8_t getValidfor() const;

	void setValidfor(uint8_t validFor);
	

	
private:
	state_data_t mSetpoint;
	uint8_t mPriority;
	uint8_t mValidfor;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_NEWSETPOINT_H_ */

