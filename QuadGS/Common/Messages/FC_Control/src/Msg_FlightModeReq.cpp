#include "Msg_FlightModeReq.h"
namespace QuadGS {


Msg_FlightModeReq::Msg_FlightModeReq(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_FlightModeReq>(msg)
{

}

Msg_FlightModeReq::Msg_FlightModeReq(const msgAddr_t destination, uint8_t mode)
:QGS_ModuleMsg<Msg_FlightModeReq>(messageTypes_t::Msg_FlightModeReq_e, destination), mMode(mode)
{

}

Msg_FlightModeReq::Msg_FlightModeReq(const Msg_FlightModeReq& msg)
:QGS_ModuleMsg<Msg_FlightModeReq>(msg), mMode(msg.mMode)
{

}

Msg_FlightModeReq::~Msg_FlightModeReq()
{

}

BinaryOStream& Msg_FlightModeReq::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mMode;
	
	return os;
}
	
BinaryIStream& Msg_FlightModeReq::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mMode;
	
	return is;
}

uint8_t Msg_FlightModeReq::getMode() const
{
	return mMode;
}
	
void Msg_FlightModeReq::setMode(uint8_t mode)
{
	mMode = mode;
}

}
