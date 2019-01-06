#include "Msg_Display.h"
namespace QuadGS {


Msg_Display::Msg_Display(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_Display>(msg)
{

}

Msg_Display::Msg_Display(const msgAddr_t destination, std::string message)
:QGS_ModuleMsg<Msg_Display>(messageTypes_t::Msg_Display_e, destination), mMessage(message)
{

}

Msg_Display::Msg_Display(const Msg_Display& msg)
:QGS_ModuleMsg<Msg_Display>(msg), mMessage(msg.mMessage)
{

}

Msg_Display::~Msg_Display()
{

}

BinaryOStream& Msg_Display::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mMessage;
	
	return os;
}
	
BinaryIStream& Msg_Display::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mMessage.erase();
	is >> mMessage;
	
	return is;
}

std::string Msg_Display::getMessage() const
{
	return mMessage;
}
	
void Msg_Display::setMessage(std::string message)
{
	mMessage = message;
}

}
