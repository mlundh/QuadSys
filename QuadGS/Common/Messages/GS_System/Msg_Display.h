#ifndef QUADGS_MESSAGE_MSG_DISPLAY_H_
#define QUADGS_MESSAGE_MSG_DISPLAY_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_Display: public QGS_ModuleMsg<Msg_Display>
{
public:

	Msg_Display(QGS_ModuleMsgBase& msg);

	Msg_Display(const msgAddr_t destination, std::string message);

	Msg_Display(const Msg_Display& msg);

	virtual ~Msg_Display();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Display> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::string getMessage() const;

	void setMessage(std::string message);
	

	
private:
	std::string mMessage;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_DISPLAY_H_ */

