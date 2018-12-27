#include "Msg_ParamIo.h"
namespace QuadGS {


Msg_ParamIo::Msg_ParamIo(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_ParamIo>(msg)
{

}

Msg_ParamIo::Msg_ParamIo(std::string destination, std::unique_ptr<QGSParamMsg> paramMsg)
:QGS_ModuleMsg<Msg_ParamIo>(messageTypes_t::Msg_ParamIo_e, destination), mParammsg(std::move(paramMsg))
{

}

Msg_ParamIo::Msg_ParamIo(const Msg_ParamIo& msg)
:QGS_ModuleMsg<Msg_ParamIo>(msg), mParammsg(std::move(msg.getCloneParammsg()))
{

}

Msg_ParamIo::~Msg_ParamIo()
{

}

BinaryOStream& Msg_ParamIo::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << *mParammsg;
	
	return os;
}
	
BinaryIStream& Msg_ParamIo::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> *mParammsg;
	
	return is;
}

std::unique_ptr<QGSParamMsg> Msg_ParamIo::getCloneParammsg() const
{
	return std::make_unique<QGSParamMsg>(*mParammsg);
}

std::unique_ptr<QGSParamMsg> Msg_ParamIo::getParammsg() 
{
	return std::move(mParammsg);
}
	
void Msg_ParamIo::setParammsg(std::unique_ptr<QGSParamMsg> paramMsg)
{
	mParammsg = std::move(paramMsg);
}

}
