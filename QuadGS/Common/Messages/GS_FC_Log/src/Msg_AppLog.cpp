#include "Msg_AppLog.h"
namespace QuadGS {


Msg_AppLog::Msg_AppLog(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_AppLog>(msg)
{

}

Msg_AppLog::Msg_AppLog(const msgAddr_t destination, uint8_t control, std::string payload)
:QGS_ModuleMsg<Msg_AppLog>(messageTypes_t::Msg_AppLog_e, destination), mControl(control), mPayload(payload)
{

}

Msg_AppLog::Msg_AppLog(const Msg_AppLog& msg)
:QGS_ModuleMsg<Msg_AppLog>(msg), mControl(msg.mControl), mPayload(msg.mPayload)
{

}

Msg_AppLog::~Msg_AppLog()
{

}

BinaryOStream& Msg_AppLog::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mControl;
	os << mPayload;
	
	return os;
}
	
BinaryIStream& Msg_AppLog::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mControl;
	mPayload.erase();
	is >> mPayload;
	
	return is;
}

uint8_t Msg_AppLog::getControl() const
{
	return mControl;
}
	
void Msg_AppLog::setControl(uint8_t control)
{
	mControl = control;
}
std::string Msg_AppLog::getPayload() const
{
	return mPayload;
}
	
void Msg_AppLog::setPayload(std::string payload)
{
	mPayload = payload;
}

}
