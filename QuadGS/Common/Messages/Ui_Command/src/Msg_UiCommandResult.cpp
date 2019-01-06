#include "Msg_UiCommandResult.h"
namespace QuadGS {


Msg_UiCommandResult::Msg_UiCommandResult(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_UiCommandResult>(msg)
{

}

Msg_UiCommandResult::Msg_UiCommandResult(const msgAddr_t destination, std::string result)
:QGS_ModuleMsg<Msg_UiCommandResult>(messageTypes_t::Msg_UiCommandResult_e, destination), mResult(result)
{

}

Msg_UiCommandResult::Msg_UiCommandResult(const Msg_UiCommandResult& msg)
:QGS_ModuleMsg<Msg_UiCommandResult>(msg), mResult(msg.mResult)
{

}

Msg_UiCommandResult::~Msg_UiCommandResult()
{

}

BinaryOStream& Msg_UiCommandResult::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mResult;
	
	return os;
}
	
BinaryIStream& Msg_UiCommandResult::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mResult.erase();
	is >> mResult;
	
	return is;
}

std::string Msg_UiCommandResult::getResult() const
{
	return mResult;
}
	
void Msg_UiCommandResult::setResult(std::string result)
{
	mResult = result;
}

}
