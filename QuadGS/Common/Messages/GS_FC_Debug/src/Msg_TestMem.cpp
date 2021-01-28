#include "Msg_TestMem.h"
namespace QuadGS {


Msg_TestMem::Msg_TestMem(QGS_ModuleMsgBase& msg)
:QGS_ModuleMsg<Msg_TestMem>(msg)
{

}

Msg_TestMem::Msg_TestMem(const msgAddr_t destination, uint8_t write, uint32_t startAddr, uint8_t startNumber, uint8_t size)
:QGS_ModuleMsg<Msg_TestMem>(messageTypes_t::Msg_TestMem_e, destination), mWrite(write), mStartaddr(startAddr), mStartnumber(startNumber), mSize(size)
{

}

Msg_TestMem::Msg_TestMem(const Msg_TestMem& msg)
:QGS_ModuleMsg<Msg_TestMem>(msg), mWrite(msg.mWrite), mStartaddr(msg.mStartaddr), mStartnumber(msg.mStartnumber), mSize(msg.mSize)
{

}

Msg_TestMem::~Msg_TestMem()
{

}

BinaryOStream& Msg_TestMem::stream(BinaryOStream& os) const
{
	QGS_ModuleMsgBase::stream(os);
	os << mWrite;
	os << mStartaddr;
	os << mStartnumber;
	os << mSize;
	
	return os;
}
	
BinaryIStream& Msg_TestMem::stream(BinaryIStream& is)
{
	QGS_ModuleMsgBase::stream(is);
	is >> mWrite;
	is >> mStartaddr;
	is >> mStartnumber;
	is >> mSize;
	
	return is;
}

uint8_t Msg_TestMem::getWrite() const
{
	return mWrite;
}
	
void Msg_TestMem::setWrite(uint8_t write)
{
	mWrite = write;
}
uint32_t Msg_TestMem::getStartaddr() const
{
	return mStartaddr;
}
	
void Msg_TestMem::setStartaddr(uint32_t startAddr)
{
	mStartaddr = startAddr;
}
uint8_t Msg_TestMem::getStartnumber() const
{
	return mStartnumber;
}
	
void Msg_TestMem::setStartnumber(uint8_t startNumber)
{
	mStartnumber = startNumber;
}
uint8_t Msg_TestMem::getSize() const
{
	return mSize;
}
	
void Msg_TestMem::setSize(uint8_t size)
{
	mSize = size;
}

}
