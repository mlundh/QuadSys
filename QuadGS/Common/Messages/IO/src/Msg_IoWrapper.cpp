#include "Msg_IoWrapper.h"
namespace QuadGS {


Msg_IoWrapper::Msg_IoWrapper(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_IoWrapper>(msg)
{

}

Msg_IoWrapper::Msg_IoWrapper(const msgAddr_t destination, std::unique_ptr<QGS_ModuleMsgBase> baseMsg)
:QGS_ModuleMsg<Msg_IoWrapper>(messageTypes_t::Msg_IoWrapper_e, destination), mBasemsg(std::move(baseMsg))
{

}

Msg_IoWrapper::Msg_IoWrapper(const Msg_IoWrapper& msg)
:QGS_ModuleMsg<Msg_IoWrapper>(msg), mBasemsg(std::move(msg.getCloneBasemsg()))
{

}

Msg_IoWrapper::~Msg_IoWrapper()
{

}

BinaryOStream& Msg_IoWrapper::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << *mBasemsg;
	
	return os;
}
	
BinaryIStream& Msg_IoWrapper::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> *mBasemsg;
	
	return is;
}

std::unique_ptr<QGS_ModuleMsgBase> Msg_IoWrapper::getCloneBasemsg() const
{
	return std::make_unique<QGS_ModuleMsgBase>(*mBasemsg);
}

std::unique_ptr<QGS_ModuleMsgBase> Msg_IoWrapper::getBasemsg() 
{
	return std::move(mBasemsg);
}
	
void Msg_IoWrapper::setBasemsg(std::unique_ptr<QGS_ModuleMsgBase> baseMsg)
{
	mBasemsg = std::move(baseMsg);
}

}
