#include "Msg_AppLog.h"
namespace QuadGS {


Msg_AppLog::Msg_AppLog(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_AppLog>(msg)
{

}

Msg_AppLog::Msg_AppLog(const msgAddr_t destination, std::string message)
:QGS_ModuleMsg<Msg_AppLog>(messageTypes_t::Msg_AppLog_e, destination), mMessage(message)
{

}

Msg_AppLog::Msg_AppLog(const Msg_AppLog& msg)
:QGS_ModuleMsg<Msg_AppLog>(msg), mMessage(msg.mMessage)
{

}

Msg_AppLog::~Msg_AppLog()
{

}

BinaryOStream& Msg_AppLog::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mMessage;
	
	return os;
}
	
BinaryIStream& Msg_AppLog::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mMessage.erase();
	is >> mMessage;
	
	return is;
}

std::string Msg_AppLog::getMessage() const
{
	return mMessage;
}
	
void Msg_AppLog::setMessage(std::string message)
{
	mMessage = message;
}

}
