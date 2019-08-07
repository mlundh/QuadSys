#ifndef QUADGS_MESSAGE_MSG_TESTTRANSMISSION_H_
#define QUADGS_MESSAGE_MSG_TESTTRANSMISSION_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_TestTransmission: public QGS_ModuleMsg<Msg_TestTransmission>
{
public:

	Msg_TestTransmission(QGS_ModuleMsgBase& msg);

	Msg_TestTransmission(const msgAddr_t destination, uint32_t test, std::string payload);

	Msg_TestTransmission(const Msg_TestTransmission& msg);

	virtual ~Msg_TestTransmission();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_TestTransmission> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint32_t getTest() const;

	void setTest(uint32_t test);
	
	std::string getPayload() const;

	void setPayload(std::string payload);
	

	
private:
	uint32_t mTest;
	std::string mPayload;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_TESTTRANSMISSION_H_ */

