#include "Msg_Debug.h"
namespace QuadGS {


Msg_Debug::Msg_Debug(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_Debug>(msg)
{

}

Msg_Debug::Msg_Debug(const msgAddr_t destination, uint8_t control, std::string payload)
:QGS_ModuleMsg<Msg_Debug>(messageTypes_t::Msg_Debug_e, destination), mControl(control), mPayload(payload)
{

}

Msg_Debug::Msg_Debug(const Msg_Debug& msg)
:QGS_ModuleMsg<Msg_Debug>(msg), mControl(msg.mControl), mPayload(msg.mPayload)
{

}

Msg_Debug::~Msg_Debug()
{

}

BinaryOStream& Msg_Debug::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mControl;
	os << mPayload;
	
	return os;
}
	
BinaryIStream& Msg_Debug::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mControl;
	mPayload.erase();
	is >> mPayload;
	
	return is;
}

uint8_t Msg_Debug::getControl() const
{
	return mControl;
}
	
void Msg_Debug::setControl(uint8_t control)
{
	mControl = control;
}
std::string Msg_Debug::getPayload() const
{
	return mPayload;
}
	
void Msg_Debug::setPayload(std::string payload)
{
	mPayload = payload;
}

}
