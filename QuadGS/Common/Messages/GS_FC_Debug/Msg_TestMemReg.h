#ifndef QUADGS_MESSAGE_MSG_TESTMEMREG_H_
#define QUADGS_MESSAGE_MSG_TESTMEMREG_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_TestMemReg: public QGS_ModuleMsg<Msg_TestMemReg>
{
public:

	Msg_TestMemReg(QGS_ModuleMsgBase& msg);

	Msg_TestMemReg(const msgAddr_t destination, uint8_t deviceId, uint8_t rdsr);

	Msg_TestMemReg(const Msg_TestMemReg& msg);

	virtual ~Msg_TestMemReg();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_TestMemReg> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint8_t getDeviceid() const;

	void setDeviceid(uint8_t deviceId);
	
	uint8_t getRdsr() const;

	void setRdsr(uint8_t rdsr);
	

	
private:
	uint8_t mDeviceid;
	uint8_t mRdsr;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_TESTMEMREG_H_ */

