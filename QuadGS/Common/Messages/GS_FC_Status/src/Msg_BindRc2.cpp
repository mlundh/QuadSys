#include "Msg_BindRc2.h"
namespace QuadGS {


Msg_BindRc2::Msg_BindRc2(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_BindRc2>(msg)
{

}

Msg_BindRc2::Msg_BindRc2(const msgAddr_t destination)
:QGS_ModuleMsg<Msg_BindRc2>(messageTypes_t::Msg_BindRc2_e, destination)
{

}

Msg_BindRc2::Msg_BindRc2(const Msg_BindRc2& msg)
:QGS_ModuleMsg<Msg_BindRc2>(msg)
{

}

Msg_BindRc2::~Msg_BindRc2()
{

}

BinaryOStream& Msg_BindRc2::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	
	return os;
}
	
BinaryIStream& Msg_BindRc2::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	
	return is;
}


}
