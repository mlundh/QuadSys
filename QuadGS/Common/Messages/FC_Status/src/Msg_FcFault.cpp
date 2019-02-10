#include "Msg_FcFault.h"
namespace QuadGS {


Msg_FcFault::Msg_FcFault(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_FcFault>(msg)
{

}

Msg_FcFault::Msg_FcFault(const msgAddr_t destination)
:QGS_ModuleMsg<Msg_FcFault>(messageTypes_t::Msg_FcFault_e, destination)
{

}

Msg_FcFault::Msg_FcFault(const Msg_FcFault& msg)
:QGS_ModuleMsg<Msg_FcFault>(msg)
{

}

Msg_FcFault::~Msg_FcFault()
{

}

BinaryOStream& Msg_FcFault::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	
	return os;
}
	
BinaryIStream& Msg_FcFault::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	
	return is;
}


}
