#ifndef QUADGS_MESSAGE_MSG_NEWRCSETPOINT_H_
#define QUADGS_MESSAGE_MSG_NEWRCSETPOINT_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_NewRcSetpoint: public QGS_ModuleMsg<Msg_NewRcSetpoint>
{
public:

	Msg_NewRcSetpoint(QGS_ModuleMsgBase& msg);

	Msg_NewRcSetpoint(const msgAddr_t destination, genericRC_t setpoint, uint8_t priority, uint8_t validFor);

	Msg_NewRcSetpoint(const Msg_NewRcSetpoint& msg);

	virtual ~Msg_NewRcSetpoint();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_NewRcSetpoint> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	genericRC_t getSetpoint() const;

	void setSetpoint(genericRC_t setpoint);
	
	uint8_t getPriority() const;

	void setPriority(uint8_t priority);
	
	uint8_t getValidfor() const;

	void setValidfor(uint8_t validFor);
	

	
private:
	genericRC_t mSetpoint;
	uint8_t mPriority;
	uint8_t mValidfor;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_NEWRCSETPOINT_H_ */

