#include "Msg_FireUiCommand.h"
namespace QuadGS {


Msg_FireUiCommand::Msg_FireUiCommand(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_FireUiCommand>(msg)
{

}

Msg_FireUiCommand::Msg_FireUiCommand(const std::string& destination, std::string command, std::string args)
:QGS_ModuleMsg<Msg_FireUiCommand>(messageTypes_t::Msg_FireUiCommand_e, destination), mCommand(command), mArgs(args)
{

}

Msg_FireUiCommand::Msg_FireUiCommand(const Msg_FireUiCommand& msg)
:QGS_ModuleMsg<Msg_FireUiCommand>(msg), mCommand(msg.mCommand), mArgs(msg.mArgs)
{

}

Msg_FireUiCommand::~Msg_FireUiCommand()
{

}

BinaryOStream& Msg_FireUiCommand::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mCommand;
	os << mArgs;
	
	return os;
}
	
BinaryIStream& Msg_FireUiCommand::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mCommand.erase();
	is >> mCommand;
	mArgs.erase();
	is >> mArgs;
	
	return is;
}

std::string Msg_FireUiCommand::getCommand() const
{
	return mCommand;
}
	
void Msg_FireUiCommand::setCommand(std::string command)
{
	mCommand = command;
}
std::string Msg_FireUiCommand::getArgs() const
{
	return mArgs;
}
	
void Msg_FireUiCommand::setArgs(std::string args)
{
	mArgs = args;
}

}
