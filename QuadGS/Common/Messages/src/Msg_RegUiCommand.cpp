#include "Msg_RegUiCommand.h"
namespace QuadGS {

Msg_RegUiCommand::Msg_RegUiCommand(std::string destination, std::string command, std::string doc)
:QGS_ModuleMsg<Msg_RegUiCommand>(messageTypes_t::Msg_RegUiCommand_e, destination), mCommand(command), mDoc(doc)
{

}

Msg_RegUiCommand::Msg_RegUiCommand(const Msg_RegUiCommand& msg)
:QGS_ModuleMsg<Msg_RegUiCommand>(msg), mCommand(msg.mCommand), mDoc(msg.mDoc)
{

}

Msg_RegUiCommand::~Msg_RegUiCommand()
{

}

BinaryOStream& Msg_RegUiCommand::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mCommand;
	os << mDoc;
	
	return os;
}
	
BinaryIStream& Msg_RegUiCommand::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	mCommand.erase();
	is >> mCommand;
	mDoc.erase();
	is >> mDoc;
	
	return is;
}

std::string Msg_RegUiCommand::getCommand() const
{
	return mCommand;
}
	
void Msg_RegUiCommand::setCommand(std::string command)
{
	mCommand = command;
}
std::string Msg_RegUiCommand::getDoc() const
{
	return mDoc;
}
	
void Msg_RegUiCommand::setDoc(std::string doc)
{
	mDoc = doc;
}

}
