#include "Msg_Error.h"
namespace QuadGS {


Msg_Error::Msg_Error(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_Error>(msg)
{

}

Msg_Error::Msg_Error(const msgAddr_t destination, std::string error)
:QGS_ModuleMsg<Msg_Error>(messageTypes_t::Msg_Error_e, destination), mError(error)
{

}

Msg_Error::Msg_Error(const Msg_Error& msg)
:QGS_ModuleMsg<Msg_Error>(msg), mError(msg.mError)
{

}

Msg_Error::~Msg_Error()
{

}

BinaryOStream& Msg_Error::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mError;
	
	return os;
}
	
BinaryIStream& Msg_Error::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mError.erase();
	is >> mError;
	
	return is;
}

std::string Msg_Error::getError() const
{
	return mError;
}
	
void Msg_Error::setError(std::string error)
{
	mError = error;
}

}
