#include "Msg_BindRc.h"
namespace QuadGS {


Msg_BindRc::Msg_BindRc(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_BindRc>(msg)
{

}

Msg_BindRc::Msg_BindRc(const msgAddr_t destination, uint8_t quit)
:QGS_ModuleMsg<Msg_BindRc>(messageTypes_t::Msg_BindRc_e, destination), mQuit(quit)
{

}

Msg_BindRc::Msg_BindRc(const Msg_BindRc& msg)
:QGS_ModuleMsg<Msg_BindRc>(msg), mQuit(msg.mQuit)
{

}

Msg_BindRc::~Msg_BindRc()
{

}

BinaryOStream& Msg_BindRc::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mQuit;
	
	return os;
}
	
BinaryIStream& Msg_BindRc::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mQuit;
	
	return is;
}

uint8_t Msg_BindRc::getQuit() const
{
	return mQuit;
}
	
void Msg_BindRc::setQuit(uint8_t quit)
{
	mQuit = quit;
}

}
