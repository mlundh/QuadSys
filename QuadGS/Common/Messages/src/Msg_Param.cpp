#include "Msg_Param.h"
namespace QuadGS {

Msg_Param::Msg_Param(std::string destination, std::unique_ptr<QGSParamMsg> paramMsg)
:QGS_ModuleMsg<Msg_Param>(messageTypes_t::Msg_Param_e, destination), mParammsg(std::move(paramMsg))
{

}

Msg_Param::Msg_Param(const Msg_Param& msg)
:QGS_ModuleMsg<Msg_Param>(msg), mParammsg(std::move(msg.getCloneParammsg()))
{

}

Msg_Param::~Msg_Param()
{

}

BinaryOStream& Msg_Param::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << *mParammsg;
		
	return os;
}
	
BinaryIStream& Msg_Param::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> *mParammsg;
		
	return is;
}

std::unique_ptr<QGSParamMsg> Msg_Param::getCloneParammsg() const
{
	return std::make_unique<QGSParamMsg>(*mParammsg);
}

std::unique_ptr<QGSParamMsg> Msg_Param::getParammsg() 
{
	return std::move(mParammsg);
}
	
void Msg_Param::setParammsg(std::unique_ptr<QGSParamMsg> paramMsg)
{
	mParammsg = std::move(paramMsg);
}

}
