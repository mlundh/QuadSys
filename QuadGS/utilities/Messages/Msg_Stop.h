#ifndef QUADGS_MESSAGE_MSG_STOP_H_
#define QUADGS_MESSAGE_MSG_STOP_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_Stop: public QGS_ModuleMsg<Msg_Stop>
{
public:

	Msg_Stop(std::string destination);

	Msg_Stop(const Msg_Stop& msg);

	virtual ~Msg_Stop();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Stop> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	
	
private:
	
};
}

#endif /* QUADGS_MESSAGE_MSG_STOP_H_ */

