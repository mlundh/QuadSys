#include "Msg_TestMemReg.h"
namespace QuadGS {


Msg_TestMemReg::Msg_TestMemReg(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_TestMemReg>(msg)
{

}

Msg_TestMemReg::Msg_TestMemReg(const msgAddr_t destination, uint8_t deviceId, uint8_t rdsr)
:QGS_ModuleMsg<Msg_TestMemReg>(messageTypes_t::Msg_TestMemReg_e, destination), mDeviceid(deviceId), mRdsr(rdsr)
{

}

Msg_TestMemReg::Msg_TestMemReg(const Msg_TestMemReg& msg)
:QGS_ModuleMsg<Msg_TestMemReg>(msg), mDeviceid(msg.mDeviceid), mRdsr(msg.mRdsr)
{

}

Msg_TestMemReg::~Msg_TestMemReg()
{

}

BinaryOStream& Msg_TestMemReg::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mDeviceid;
	os << mRdsr;
	
	return os;
}
	
BinaryIStream& Msg_TestMemReg::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mDeviceid;
	is >> mRdsr;
	
	return is;
}

uint8_t Msg_TestMemReg::getDeviceid() const
{
	return mDeviceid;
}
	
void Msg_TestMemReg::setDeviceid(uint8_t deviceId)
{
	mDeviceid = deviceId;
}
uint8_t Msg_TestMemReg::getRdsr() const
{
	return mRdsr;
}
	
void Msg_TestMemReg::setRdsr(uint8_t rdsr)
{
	mRdsr = rdsr;
}

}
