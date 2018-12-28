#include "Msg_DebugIo.h"
namespace QuadGS {


Msg_DebugIo::Msg_DebugIo(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_DebugIo>(msg)
{

}

Msg_DebugIo::Msg_DebugIo(const msgAddr_t destination, std::unique_ptr<QGS_DebugMsg> dbgMsg)
:QGS_ModuleMsg<Msg_DebugIo>(messageTypes_t::Msg_DebugIo_e, destination), mDbgmsg(std::move(dbgMsg))
{

}

Msg_DebugIo::Msg_DebugIo(const Msg_DebugIo& msg)
:QGS_ModuleMsg<Msg_DebugIo>(msg), mDbgmsg(std::move(msg.getCloneDbgmsg()))
{

}

Msg_DebugIo::~Msg_DebugIo()
{

}

BinaryOStream& Msg_DebugIo::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << *mDbgmsg;
	
	return os;
}
	
BinaryIStream& Msg_DebugIo::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> *mDbgmsg;
	
	return is;
}

std::unique_ptr<QGS_DebugMsg> Msg_DebugIo::getCloneDbgmsg() const
{
	return std::make_unique<QGS_DebugMsg>(*mDbgmsg);
}

std::unique_ptr<QGS_DebugMsg> Msg_DebugIo::getDbgmsg() 
{
	return std::move(mDbgmsg);
}
	
void Msg_DebugIo::setDbgmsg(std::unique_ptr<QGS_DebugMsg> dbgMsg)
{
	mDbgmsg = std::move(dbgMsg);
}

}
