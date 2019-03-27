#include "Msg_ParamSave.h"
namespace QuadGS {


Msg_ParamSave::Msg_ParamSave(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_ParamSave>(msg)
{

}

Msg_ParamSave::Msg_ParamSave(const msgAddr_t destination)
:QGS_ModuleMsg<Msg_ParamSave>(messageTypes_t::Msg_ParamSave_e, destination)
{

}

Msg_ParamSave::Msg_ParamSave(const Msg_ParamSave& msg)
:QGS_ModuleMsg<Msg_ParamSave>(msg)
{

}

Msg_ParamSave::~Msg_ParamSave()
{

}

BinaryOStream& Msg_ParamSave::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	
	return os;
}
	
BinaryIStream& Msg_ParamSave::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	
	return is;
}


}
