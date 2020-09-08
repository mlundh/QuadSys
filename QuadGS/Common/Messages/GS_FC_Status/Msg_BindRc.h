#ifndef QUADGS_MESSAGE_MSG_BINDRC_H_
#define QUADGS_MESSAGE_MSG_BINDRC_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_BindRc: public QGS_ModuleMsg<Msg_BindRc>
{
public:

	Msg_BindRc(QGS_ModuleMsgBase& msg);

	Msg_BindRc(const msgAddr_t destination, uint8_t quit);

	Msg_BindRc(const Msg_BindRc& msg);

	virtual ~Msg_BindRc();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_BindRc> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint8_t getQuit() const;

	void setQuit(uint8_t quit);
	

	
private:
	uint8_t mQuit;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_BINDRC_H_ */

