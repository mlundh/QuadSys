#include "Msg_Test.h"
namespace QuadGS {


Msg_Test::Msg_Test(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_Test>(msg)
{

}

Msg_Test::Msg_Test(const msgAddr_t destination)
:QGS_ModuleMsg<Msg_Test>(messageTypes_t::Msg_Test_e, destination)
{

}

Msg_Test::Msg_Test(const Msg_Test& msg)
:QGS_ModuleMsg<Msg_Test>(msg)
{

}

Msg_Test::~Msg_Test()
{

}

BinaryOStream& Msg_Test::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	
	return os;
}
	
BinaryIStream& Msg_Test::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	
	return is;
}


}
