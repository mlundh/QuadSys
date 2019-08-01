#include "Msg_TestTransmission.h"
namespace QuadGS {


Msg_TestTransmission::Msg_TestTransmission(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_TestTransmission>(msg)
{

}

Msg_TestTransmission::Msg_TestTransmission(const msgAddr_t destination, uint32_t test, std::string payload)
:QGS_ModuleMsg<Msg_TestTransmission>(messageTypes_t::Msg_TestTransmission_e, destination), mTest(test), mPayload(payload)
{

}

Msg_TestTransmission::Msg_TestTransmission(const Msg_TestTransmission& msg)
:QGS_ModuleMsg<Msg_TestTransmission>(msg), mTest(msg.mTest), mPayload(msg.mPayload)
{

}

Msg_TestTransmission::~Msg_TestTransmission()
{

}

BinaryOStream& Msg_TestTransmission::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mTest;
	os << mPayload;
	
	return os;
}
	
BinaryIStream& Msg_TestTransmission::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mTest;
	mPayload.erase();
	is >> mPayload;
	
	return is;
}

uint32_t Msg_TestTransmission::getTest() const
{
	return mTest;
}
	
void Msg_TestTransmission::setTest(uint32_t test)
{
	mTest = test;
}
std::string Msg_TestTransmission::getPayload() const
{
	return mPayload;
}
	
void Msg_TestTransmission::setPayload(std::string payload)
{
	mPayload = payload;
}

}
