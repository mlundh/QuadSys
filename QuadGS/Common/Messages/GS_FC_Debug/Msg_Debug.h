#ifndef QUADGS_MESSAGE_MSG_DEBUG_H_
#define QUADGS_MESSAGE_MSG_DEBUG_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_Debug: public QGS_ModuleMsg<Msg_Debug>
{
public:

	Msg_Debug(QGS_ModuleMsgBase& msg);

	Msg_Debug(const msgAddr_t destination, uint8_t control, std::string payload);

	Msg_Debug(const Msg_Debug& msg);

	virtual ~Msg_Debug();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Debug> ptr;

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

#endif /* QUADGS_MESSAGE_MSG_DEBUG_H_ */

