#include "Msg_GsAppLogFind.h"
namespace QuadGS {


Msg_GsAppLogFind::Msg_GsAppLogFind(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_GsAppLogFind>(msg)
{

}

Msg_GsAppLogFind::Msg_GsAppLogFind(const msgAddr_t destination, uint8_t command, std::string toFind, std::string found)
:QGS_ModuleMsg<Msg_GsAppLogFind>(messageTypes_t::Msg_GsAppLogFind_e, destination), mCommand(command), mTofind(toFind), mFound(found)
{

}

Msg_GsAppLogFind::Msg_GsAppLogFind(const Msg_GsAppLogFind& msg)
:QGS_ModuleMsg<Msg_GsAppLogFind>(msg), mCommand(msg.mCommand), mTofind(msg.mTofind), mFound(msg.mFound)
{

}

Msg_GsAppLogFind::~Msg_GsAppLogFind()
{

}

BinaryOStream& Msg_GsAppLogFind::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mCommand;
	os << mTofind;
	os << mFound;
	
	return os;
}
	
BinaryIStream& Msg_GsAppLogFind::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mCommand;
	mTofind.erase();
	is >> mTofind;
	mFound.erase();
	is >> mFound;
	
	return is;
}

uint8_t Msg_GsAppLogFind::getCommand() const
{
	return mCommand;
}
	
void Msg_GsAppLogFind::setCommand(uint8_t command)
{
	mCommand = command;
}
std::string Msg_GsAppLogFind::getTofind() const
{
	return mTofind;
}
	
void Msg_GsAppLogFind::setTofind(std::string toFind)
{
	mTofind = toFind;
}
std::string Msg_GsAppLogFind::getFound() const
{
	return mFound;
}
	
void Msg_GsAppLogFind::setFound(std::string found)
{
	mFound = found;
}

}
