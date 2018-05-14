#include "Msg_Debug.h"
namespace QuadGS {

Msg_Debug::Msg_Debug(std::string destination, std::unique_ptr<QGS_DebugMsg> dbgMsg)
:QGS_ModuleMsg<Msg_Debug>(messageTypes_t::Msg_Debug_e, destination), mDbgmsg(std::move(dbgMsg))
{

}

Msg_Debug::Msg_Debug(const Msg_Debug& msg)
:QGS_ModuleMsg<Msg_Debug>(msg), mDbgmsg(std::move(msg.getCloneDbgmsg()))
{

}

Msg_Debug::~Msg_Debug()
{

}

BinaryOStream& Msg_Debug::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << *mDbgmsg;
		
	return os;
}
	
BinaryIStream& Msg_Debug::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> *mDbgmsg;
		
	return is;
}

std::unique_ptr<QGS_DebugMsg> Msg_Debug::getCloneDbgmsg() const
{
	return std::make_unique<QGS_DebugMsg>(*mDbgmsg);
}

std::unique_ptr<QGS_DebugMsg> Msg_Debug::getDbgmsg() 
{
	return std::move(mDbgmsg);
}
	
void Msg_Debug::setDbgmsg(std::unique_ptr<QGS_DebugMsg> dbgMsg)
{
	mDbgmsg = std::move(dbgMsg);
}

}
