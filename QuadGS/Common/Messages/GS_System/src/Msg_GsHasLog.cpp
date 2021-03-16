#include "Msg_GsHasLog.h"
namespace QuadGS {


Msg_GsHasLog::Msg_GsHasLog(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_GsHasLog>(msg)
{

}

Msg_GsHasLog::Msg_GsHasLog(const msgAddr_t destination)
:QGS_ModuleMsg<Msg_GsHasLog>(messageTypes_t::Msg_GsHasLog_e, destination)
{

}

Msg_GsHasLog::Msg_GsHasLog(const Msg_GsHasLog& msg)
:QGS_ModuleMsg<Msg_GsHasLog>(msg)
{

}

Msg_GsHasLog::~Msg_GsHasLog()
{

}

BinaryOStream& Msg_GsHasLog::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	
	return os;
}
	
BinaryIStream& Msg_GsHasLog::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	
	return is;
}


}
