#include "Msg_LogIo.h"
namespace QuadGS {


Msg_LogIo::Msg_LogIo(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_LogIo>(msg)
{

}

Msg_LogIo::Msg_LogIo(const msgAddr_t destination, std::unique_ptr<QGS_LogMsg> paramMsg)
:QGS_ModuleMsg<Msg_LogIo>(messageTypes_t::Msg_LogIo_e, destination), mParammsg(std::move(paramMsg))
{

}

Msg_LogIo::Msg_LogIo(const Msg_LogIo& msg)
:QGS_ModuleMsg<Msg_LogIo>(msg), mParammsg(std::move(msg.getCloneParammsg()))
{

}

Msg_LogIo::~Msg_LogIo()
{

}

BinaryOStream& Msg_LogIo::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << *mParammsg;
	
	return os;
}
	
BinaryIStream& Msg_LogIo::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> *mParammsg;
	
	return is;
}

std::unique_ptr<QGS_LogMsg> Msg_LogIo::getCloneParammsg() const
{
	return std::make_unique<QGS_LogMsg>(*mParammsg);
}

std::unique_ptr<QGS_LogMsg> Msg_LogIo::getParammsg() 
{
	return std::move(mParammsg);
}
	
void Msg_LogIo::setParammsg(std::unique_ptr<QGS_LogMsg> paramMsg)
{
	mParammsg = std::move(paramMsg);
}

}
