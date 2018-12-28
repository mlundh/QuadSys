#include "Msg_RegisterName.h"
namespace QuadGS {


Msg_RegisterName::Msg_RegisterName(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_RegisterName>(msg)
{

}

Msg_RegisterName::Msg_RegisterName(const std::string& destination, std::string name)
:QGS_ModuleMsg<Msg_RegisterName>(messageTypes_t::Msg_RegisterName_e, destination), mName(name)
{

}

Msg_RegisterName::Msg_RegisterName(const Msg_RegisterName& msg)
:QGS_ModuleMsg<Msg_RegisterName>(msg), mName(msg.mName)
{

}

Msg_RegisterName::~Msg_RegisterName()
{

}

BinaryOStream& Msg_RegisterName::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mName;
	
	return os;
}
	
BinaryIStream& Msg_RegisterName::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mName.erase();
	is >> mName;
	
	return is;
}

std::string Msg_RegisterName::getName() const
{
	return mName;
}
	
void Msg_RegisterName::setName(std::string name)
{
	mName = name;
}

}
