#include "Msg_Log.h"
namespace QuadGS {

Msg_Log::Msg_Log(std::string destination, std::unique_ptr<QGS_LogMsg> paramMsg)
:QGS_ModuleMsg<Msg_Log>(messageTypes_t::Msg_Log_e, destination), mParammsg(std::move(paramMsg))
{

}

Msg_Log::Msg_Log(const Msg_Log& msg)
:QGS_ModuleMsg<Msg_Log>(msg), mParammsg(std::move(msg.getCloneParammsg()))
{

}

Msg_Log::~Msg_Log()
{

}

BinaryOStream& Msg_Log::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << *mParammsg;
		
	return os;
}
	
BinaryIStream& Msg_Log::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> *mParammsg;
		
	return is;
}

std::unique_ptr<QGS_LogMsg> Msg_Log::getCloneParammsg() const
{
	return std::make_unique<QGS_LogMsg>(*mParammsg);
}

std::unique_ptr<QGS_LogMsg> Msg_Log::getParammsg() 
{
	return std::move(mParammsg);
}
	
void Msg_Log::setParammsg(std::unique_ptr<QGS_LogMsg> paramMsg)
{
	mParammsg = std::move(paramMsg);
}

}
