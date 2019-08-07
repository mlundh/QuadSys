#include "Msg_FlightMode.h"
namespace QuadGS {


Msg_FlightMode::Msg_FlightMode(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_FlightMode>(msg)
{

}

Msg_FlightMode::Msg_FlightMode(const msgAddr_t destination, uint8_t mode)
:QGS_ModuleMsg<Msg_FlightMode>(messageTypes_t::Msg_FlightMode_e, destination), mMode(mode)
{

}

Msg_FlightMode::Msg_FlightMode(const Msg_FlightMode& msg)
:QGS_ModuleMsg<Msg_FlightMode>(msg), mMode(msg.mMode)
{

}

Msg_FlightMode::~Msg_FlightMode()
{

}

BinaryOStream& Msg_FlightMode::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mMode;
	
	return os;
}
	
BinaryIStream& Msg_FlightMode::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mMode;
	
	return is;
}

uint8_t Msg_FlightMode::getMode() const
{
	return mMode;
}
	
void Msg_FlightMode::setMode(uint8_t mode)
{
	mMode = mode;
}

}
