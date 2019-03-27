#include "Msg_ParamLoad.h"
namespace QuadGS {


Msg_ParamLoad::Msg_ParamLoad(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_ParamLoad>(msg)
{

}

Msg_ParamLoad::Msg_ParamLoad(const msgAddr_t destination)
:QGS_ModuleMsg<Msg_ParamLoad>(messageTypes_t::Msg_ParamLoad_e, destination)
{

}

Msg_ParamLoad::Msg_ParamLoad(const Msg_ParamLoad& msg)
:QGS_ModuleMsg<Msg_ParamLoad>(msg)
{

}

Msg_ParamLoad::~Msg_ParamLoad()
{

}

BinaryOStream& Msg_ParamLoad::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	
	return os;
}
	
BinaryIStream& Msg_ParamLoad::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	
	return is;
}


}
