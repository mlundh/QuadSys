#include "Msg_CtrlMode.h"
namespace QuadGS {


Msg_CtrlMode::Msg_CtrlMode(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_CtrlMode>(msg)
{

}

Msg_CtrlMode::Msg_CtrlMode(const msgAddr_t destination, uint8_t mode)
:QGS_ModuleMsg<Msg_CtrlMode>(messageTypes_t::Msg_CtrlMode_e, destination), mMode(mode)
{

}

Msg_CtrlMode::Msg_CtrlMode(const Msg_CtrlMode& msg)
:QGS_ModuleMsg<Msg_CtrlMode>(msg), mMode(msg.mMode)
{

}

Msg_CtrlMode::~Msg_CtrlMode()
{

}

BinaryOStream& Msg_CtrlMode::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mMode;
	
	return os;
}
	
BinaryIStream& Msg_CtrlMode::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mMode;
	
	return is;
}

uint8_t Msg_CtrlMode::getMode() const
{
	return mMode;
}
	
void Msg_CtrlMode::setMode(uint8_t mode)
{
	mMode = mode;
}

}
