#include "Msg_NewRcSetpoint.h"
namespace QuadGS {


Msg_NewRcSetpoint::Msg_NewRcSetpoint(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_NewRcSetpoint>(msg)
{

}

Msg_NewRcSetpoint::Msg_NewRcSetpoint(const msgAddr_t destination, genericRC_t setpoint, uint8_t priority, uint8_t validFor)
:QGS_ModuleMsg<Msg_NewRcSetpoint>(messageTypes_t::Msg_NewRcSetpoint_e, destination), mSetpoint(setpoint), mPriority(priority), mValidfor(validFor)
{

}

Msg_NewRcSetpoint::Msg_NewRcSetpoint(const Msg_NewRcSetpoint& msg)
:QGS_ModuleMsg<Msg_NewRcSetpoint>(msg), mSetpoint(msg.mSetpoint), mPriority(msg.mPriority), mValidfor(msg.mValidfor)
{

}

Msg_NewRcSetpoint::~Msg_NewRcSetpoint()
{

}

BinaryOStream& Msg_NewRcSetpoint::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mSetpoint;
	os << mPriority;
	os << mValidfor;
	
	return os;
}
	
BinaryIStream& Msg_NewRcSetpoint::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mSetpoint;
	is >> mPriority;
	is >> mValidfor;
	
	return is;
}

genericRC_t Msg_NewRcSetpoint::getSetpoint() const
{
	return mSetpoint;
}
	
void Msg_NewRcSetpoint::setSetpoint(genericRC_t setpoint)
{
	mSetpoint = setpoint;
}
uint8_t Msg_NewRcSetpoint::getPriority() const
{
	return mPriority;
}
	
void Msg_NewRcSetpoint::setPriority(uint8_t priority)
{
	mPriority = priority;
}
uint8_t Msg_NewRcSetpoint::getValidfor() const
{
	return mValidfor;
}
	
void Msg_NewRcSetpoint::setValidfor(uint8_t validFor)
{
	mValidfor = validFor;
}

}
