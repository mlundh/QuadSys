#include "Msg_Transmission.h"
namespace QuadGS {


Msg_Transmission::Msg_Transmission(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_Transmission>(msg)
{

}

Msg_Transmission::Msg_Transmission(const std::string& destination, uint8_t status)
:QGS_ModuleMsg<Msg_Transmission>(messageTypes_t::Msg_Transmission_e, destination), mStatus(status)
{

}

Msg_Transmission::Msg_Transmission(const Msg_Transmission& msg)
:QGS_ModuleMsg<Msg_Transmission>(msg), mStatus(msg.mStatus)
{

}

Msg_Transmission::~Msg_Transmission()
{

}

BinaryOStream& Msg_Transmission::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mStatus;
	
	return os;
}
	
BinaryIStream& Msg_Transmission::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mStatus;
	
	return is;
}

uint8_t Msg_Transmission::getStatus() const
{
	return mStatus;
}
	
void Msg_Transmission::setStatus(uint8_t status)
{
	mStatus = status;
}

}
