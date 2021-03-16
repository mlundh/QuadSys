#include "Msg_GsAppLog.h"
namespace QuadGS {


Msg_GsAppLog::Msg_GsAppLog(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_GsAppLog>(msg)
{

}

Msg_GsAppLog::Msg_GsAppLog(const msgAddr_t destination, std::string message)
:QGS_ModuleMsg<Msg_GsAppLog>(messageTypes_t::Msg_GsAppLog_e, destination), mMessage(message)
{

}

Msg_GsAppLog::Msg_GsAppLog(const Msg_GsAppLog& msg)
:QGS_ModuleMsg<Msg_GsAppLog>(msg), mMessage(msg.mMessage)
{

}

Msg_GsAppLog::~Msg_GsAppLog()
{

}

BinaryOStream& Msg_GsAppLog::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mMessage;
	
	return os;
}
	
BinaryIStream& Msg_GsAppLog::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mMessage.erase();
	is >> mMessage;
	
	return is;
}

std::string Msg_GsAppLog::getMessage() const
{
	return mMessage;
}
	
void Msg_GsAppLog::setMessage(std::string message)
{
	mMessage = message;
}

}
