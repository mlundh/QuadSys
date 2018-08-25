#include "Msg_FindParam.h"
namespace QuadGS {

Msg_FindParam::Msg_FindParam(std::string destination, std::string toFind, std::string found)
:QGS_ModuleMsg<Msg_FindParam>(messageTypes_t::Msg_FindParam_e, destination), mTofind(toFind), mFound(found)
{

}

Msg_FindParam::Msg_FindParam(const Msg_FindParam& msg)
:QGS_ModuleMsg<Msg_FindParam>(msg), mTofind(msg.mTofind), mFound(msg.mFound)
{

}

Msg_FindParam::~Msg_FindParam()
{

}

BinaryOStream& Msg_FindParam::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mTofind;
	os << mFound;
	
	return os;
}
	
BinaryIStream& Msg_FindParam::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mTofind.erase();
	is >> mTofind;
	mFound.erase();
	is >> mFound;
	
	return is;
}

std::string Msg_FindParam::getTofind() const
{
	return mTofind;
}
	
void Msg_FindParam::setTofind(std::string toFind)
{
	mTofind = toFind;
}
std::string Msg_FindParam::getFound() const
{
	return mFound;
}
	
void Msg_FindParam::setFound(std::string found)
{
	mFound = found;
}

}
