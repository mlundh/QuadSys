#ifndef QUADGS_MESSAGE_MSG_LOG_H_
#define QUADGS_MESSAGE_MSG_LOG_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_Log: public QGS_ModuleMsg<Msg_Log>
{
public:

	Msg_Log(QGS_ModuleMsgBase& msg);

	Msg_Log(const std::string& destination, uint8_t control, std::string payload);

	Msg_Log(const Msg_Log& msg);

	virtual ~Msg_Log();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Log> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
		uint8_t getControl() const;

	void setControl(uint8_t control);
	std::string getPayload() const;

	void setPayload(std::string payload);

	
private:
	uint8_t mControl;
	std::string mPayload;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_LOG_H_ */

