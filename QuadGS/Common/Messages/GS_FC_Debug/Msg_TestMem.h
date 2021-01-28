#ifndef QUADGS_MESSAGE_MSG_TESTMEM_H_
#define QUADGS_MESSAGE_MSG_TESTMEM_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_TestMem: public QGS_ModuleMsg<Msg_TestMem>
{
public:

	Msg_TestMem(QGS_ModuleMsgBase& msg);

	Msg_TestMem(const msgAddr_t destination, uint8_t write, uint32_t startAddr, uint8_t startNumber, uint8_t size);

	Msg_TestMem(const Msg_TestMem& msg);

	virtual ~Msg_TestMem();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_TestMem> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint8_t getWrite() const;

	void setWrite(uint8_t write);
	
	uint32_t getStartaddr() const;

	void setStartaddr(uint32_t startAddr);
	
	uint8_t getStartnumber() const;

	void setStartnumber(uint8_t startNumber);
	
	uint8_t getSize() const;

	void setSize(uint8_t size);
	

	
private:
	uint8_t mWrite;
	uint32_t mStartaddr;
	uint8_t mStartnumber;
	uint8_t mSize;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_TESTMEM_H_ */

