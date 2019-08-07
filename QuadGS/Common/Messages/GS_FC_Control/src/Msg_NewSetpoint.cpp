#include "Msg_NewSetpoint.h"
namespace QuadGS {


Msg_NewSetpoint::Msg_NewSetpoint(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_NewSetpoint>(msg)
{

}

Msg_NewSetpoint::Msg_NewSetpoint(const msgAddr_t destination, state_data_t setpoint, uint8_t priority, uint8_t validFor)
:QGS_ModuleMsg<Msg_NewSetpoint>(messageTypes_t::Msg_NewSetpoint_e, destination), mSetpoint(setpoint), mPriority(priority), mValidfor(validFor)
{

}

Msg_NewSetpoint::Msg_NewSetpoint(const Msg_NewSetpoint& msg)
:QGS_ModuleMsg<Msg_NewSetpoint>(msg), mSetpoint(msg.mSetpoint), mPriority(msg.mPriority), mValidfor(msg.mValidfor)
{

}

Msg_NewSetpoint::~Msg_NewSetpoint()
{

}

BinaryOStream& Msg_NewSetpoint::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mSetpoint;
	os << mPriority;
	os << mValidfor;
	
	return os;
}
	
BinaryIStream& Msg_NewSetpoint::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mSetpoint;
	is >> mPriority;
	is >> mValidfor;
	
	return is;
}

state_data_t Msg_NewSetpoint::getSetpoint() const
{
	return mSetpoint;
}
	
void Msg_NewSetpoint::setSetpoint(state_data_t setpoint)
{
	mSetpoint = setpoint;
}
uint8_t Msg_NewSetpoint::getPriority() const
{
	return mPriority;
}
	
void Msg_NewSetpoint::setPriority(uint8_t priority)
{
	mPriority = priority;
}
uint8_t Msg_NewSetpoint::getValidfor() const
{
	return mValidfor;
}
	
void Msg_NewSetpoint::setValidfor(uint8_t validFor)
{
	mValidfor = validFor;
}

}
