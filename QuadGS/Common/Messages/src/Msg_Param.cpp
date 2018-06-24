#include "Msg_Param.h"
namespace QuadGS {

Msg_Param::Msg_Param(std::string destination, uint8_t control, uint8_t sequenceNr, uint8_t lastInSequence, std::string payload)
:QGS_ModuleMsg<Msg_Param>(messageTypes_t::Msg_Param_e, destination), mControl(control), mSequencenr(sequenceNr), mLastinsequence(lastInSequence), mPayload(payload)
{

}

Msg_Param::Msg_Param(const Msg_Param& msg)
:QGS_ModuleMsg<Msg_Param>(msg), mControl(msg.mControl), mSequencenr(msg.mSequencenr), mLastinsequence(msg.mLastinsequence), mPayload(msg.mPayload)
{

}

Msg_Param::~Msg_Param()
{

}

BinaryOStream& Msg_Param::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mControl;
	os << mSequencenr;
	os << mLastinsequence;
	os << mPayload;
	
	return os;
}
	
BinaryIStream& Msg_Param::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mControl;
	is >> mSequencenr;
	is >> mLastinsequence;
	mPayload.erase();
	is >> mPayload;
	
	return is;
}

uint8_t Msg_Param::getControl() const
{
	return mControl;
}
	
void Msg_Param::setControl(uint8_t control)
{
	mControl = control;
}
uint8_t Msg_Param::getSequencenr() const
{
	return mSequencenr;
}
	
void Msg_Param::setSequencenr(uint8_t sequenceNr)
{
	mSequencenr = sequenceNr;
}
uint8_t Msg_Param::getLastinsequence() const
{
	return mLastinsequence;
}
	
void Msg_Param::setLastinsequence(uint8_t lastInSequence)
{
	mLastinsequence = lastInSequence;
}
std::string Msg_Param::getPayload() const
{
	return mPayload;
}
	
void Msg_Param::setPayload(std::string payload)
{
	mPayload = payload;
}

}
