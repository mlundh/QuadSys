#include "Msg_CtrlModeReq.h"
namespace QuadGS {


Msg_CtrlModeReq::Msg_CtrlModeReq(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_CtrlModeReq>(msg)
{

}

Msg_CtrlModeReq::Msg_CtrlModeReq(const msgAddr_t destination, uint8_t mode)
:QGS_ModuleMsg<Msg_CtrlModeReq>(messageTypes_t::Msg_CtrlModeReq_e, destination), mMode(mode)
{

}

Msg_CtrlModeReq::Msg_CtrlModeReq(const Msg_CtrlModeReq& msg)
:QGS_ModuleMsg<Msg_CtrlModeReq>(msg), mMode(msg.mMode)
{

}

Msg_CtrlModeReq::~Msg_CtrlModeReq()
{

}

BinaryOStream& Msg_CtrlModeReq::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mMode;
	
	return os;
}
	
BinaryIStream& Msg_CtrlModeReq::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mMode;
	
	return is;
}

uint8_t Msg_CtrlModeReq::getMode() const
{
	return mMode;
}
	
void Msg_CtrlModeReq::setMode(uint8_t mode)
{
	mMode = mode;
}

}
