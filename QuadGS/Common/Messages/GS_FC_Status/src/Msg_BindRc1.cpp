#include "Msg_BindRc1.h"
namespace QuadGS {


Msg_BindRc1::Msg_BindRc1(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_BindRc1>(msg)
{

}

Msg_BindRc1::Msg_BindRc1(const msgAddr_t destination)
:QGS_ModuleMsg<Msg_BindRc1>(messageTypes_t::Msg_BindRc1_e, destination)
{

}

Msg_BindRc1::Msg_BindRc1(const Msg_BindRc1& msg)
:QGS_ModuleMsg<Msg_BindRc1>(msg)
{

}

Msg_BindRc1::~Msg_BindRc1()
{

}

BinaryOStream& Msg_BindRc1::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	
	return os;
}
	
BinaryIStream& Msg_BindRc1::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	
	return is;
}


}
