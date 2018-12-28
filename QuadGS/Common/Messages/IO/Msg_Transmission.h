#ifndef QUADGS_MESSAGE_MSG_TRANSMISSION_H_
#define QUADGS_MESSAGE_MSG_TRANSMISSION_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_Transmission: public QGS_ModuleMsg<Msg_Transmission>
{
public:

	Msg_Transmission(QGS_ModuleMsgBase& msg);

	Msg_Transmission(const std::string& destination, uint8_t status);

	Msg_Transmission(const Msg_Transmission& msg);

	virtual ~Msg_Transmission();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Transmission> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
		uint8_t getStatus() const;

	void setStatus(uint8_t status);

	
private:
	uint8_t mStatus;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_TRANSMISSION_H_ */

