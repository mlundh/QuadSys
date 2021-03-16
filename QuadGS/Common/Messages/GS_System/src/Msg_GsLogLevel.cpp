#include "Msg_GsLogLevel.h"
namespace QuadGS {


Msg_GsLogLevel::Msg_GsLogLevel(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_GsLogLevel>(msg)
{

}

Msg_GsLogLevel::Msg_GsLogLevel(const msgAddr_t destination, uint8_t command, uint32_t logLevel)
:QGS_ModuleMsg<Msg_GsLogLevel>(messageTypes_t::Msg_GsLogLevel_e, destination), mCommand(command), mLoglevel(logLevel)
{

}

Msg_GsLogLevel::Msg_GsLogLevel(const Msg_GsLogLevel& msg)
:QGS_ModuleMsg<Msg_GsLogLevel>(msg), mCommand(msg.mCommand), mLoglevel(msg.mLoglevel)
{

}

Msg_GsLogLevel::~Msg_GsLogLevel()
{

}

BinaryOStream& Msg_GsLogLevel::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mCommand;
	os << mLoglevel;
	
	return os;
}
	
BinaryIStream& Msg_GsLogLevel::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mCommand;
	is >> mLoglevel;
	
	return is;
}

uint8_t Msg_GsLogLevel::getCommand() const
{
	return mCommand;
}
	
void Msg_GsLogLevel::setCommand(uint8_t command)
{
	mCommand = command;
}
uint32_t Msg_GsLogLevel::getLoglevel() const
{
	return mLoglevel;
}
	
void Msg_GsLogLevel::setLoglevel(uint32_t logLevel)
{
	mLoglevel = logLevel;
}

}
