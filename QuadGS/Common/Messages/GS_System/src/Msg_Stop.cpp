#include "Msg_Stop.h"
namespace QuadGS {


Msg_Stop::Msg_Stop(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_Stop>(msg)
{

}

Msg_Stop::Msg_Stop(std::string destination)
:QGS_ModuleMsg<Msg_Stop>(messageTypes_t::Msg_Stop_e, destination)
{

}

Msg_Stop::Msg_Stop(const Msg_Stop& msg)
:QGS_ModuleMsg<Msg_Stop>(msg)
{

}

Msg_Stop::~Msg_Stop()
{

}

BinaryOStream& Msg_Stop::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	
	return os;
}
	
BinaryIStream& Msg_Stop::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	
	return is;
}


}
