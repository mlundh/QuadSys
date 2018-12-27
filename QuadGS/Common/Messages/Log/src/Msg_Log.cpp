#include "Msg_Log.h"
namespace QuadGS {


Msg_Log::Msg_Log(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_Log>(msg)
{

}

Msg_Log::Msg_Log(std::string destination, uint8_t control, std::string payload)
:QGS_ModuleMsg<Msg_Log>(messageTypes_t::Msg_Log_e, destination), mControl(control), mPayload(payload)
{

}

Msg_Log::Msg_Log(const Msg_Log& msg)
:QGS_ModuleMsg<Msg_Log>(msg), mControl(msg.mControl), mPayload(msg.mPayload)
{

}

Msg_Log::~Msg_Log()
{

}

BinaryOStream& Msg_Log::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mControl;
	os << mPayload;
	
	return os;
}
	
BinaryIStream& Msg_Log::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mControl;
	mPayload.erase();
	is >> mPayload;
	
	return is;
}

uint8_t Msg_Log::getControl() const
{
	return mControl;
}
	
void Msg_Log::setControl(uint8_t control)
{
	mControl = control;
}
std::string Msg_Log::getPayload() const
{
	return mPayload;
}
	
void Msg_Log::setPayload(std::string payload)
{
	mPayload = payload;
}

}
