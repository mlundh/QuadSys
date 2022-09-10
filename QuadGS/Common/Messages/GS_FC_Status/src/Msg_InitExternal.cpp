#include "Msg_InitExternal.h"
namespace QuadGS {


Msg_InitExternal::Msg_InitExternal(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_InitExternal>(msg)
{

}

Msg_InitExternal::Msg_InitExternal(const msgAddr_t destination, std::string init)
:QGS_ModuleMsg<Msg_InitExternal>(messageTypes_t::Msg_InitExternal_e, destination), mInit(init)
{

}

Msg_InitExternal::Msg_InitExternal(const Msg_InitExternal& msg)
:QGS_ModuleMsg<Msg_InitExternal>(msg), mInit(msg.mInit)
{

}

Msg_InitExternal::~Msg_InitExternal()
{

}

BinaryOStream& Msg_InitExternal::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mInit;
	
	return os;
}
	
BinaryIStream& Msg_InitExternal::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mInit.erase();
	is >> mInit;
	
	return is;
}

std::string Msg_InitExternal::getInit() const
{
	return mInit;
}
	
void Msg_InitExternal::setInit(std::string init)
{
	mInit = init;
}

}
