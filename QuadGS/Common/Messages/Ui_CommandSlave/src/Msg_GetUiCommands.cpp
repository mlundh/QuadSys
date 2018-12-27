#include "Msg_GetUiCommands.h"
namespace QuadGS {


Msg_GetUiCommands::Msg_GetUiCommands(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_GetUiCommands>(msg)
{

}

Msg_GetUiCommands::Msg_GetUiCommands(std::string destination)
:QGS_ModuleMsg<Msg_GetUiCommands>(messageTypes_t::Msg_GetUiCommands_e, destination)
{

}

Msg_GetUiCommands::Msg_GetUiCommands(const Msg_GetUiCommands& msg)
:QGS_ModuleMsg<Msg_GetUiCommands>(msg)
{

}

Msg_GetUiCommands::~Msg_GetUiCommands()
{

}

BinaryOStream& Msg_GetUiCommands::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	
	return os;
}
	
BinaryIStream& Msg_GetUiCommands::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	
	return is;
}


}
