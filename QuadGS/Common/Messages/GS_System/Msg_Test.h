#ifndef QUADGS_MESSAGE_MSG_TEST_H_
#define QUADGS_MESSAGE_MSG_TEST_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_Test: public QGS_ModuleMsg<Msg_Test>
{
public:

	Msg_Test(QGS_ModuleMsgBase& msg);

	Msg_Test(const msgAddr_t destination);

	Msg_Test(const Msg_Test& msg);

	virtual ~Msg_Test();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Test> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	

	
private:
	
};
}

#endif /* QUADGS_MESSAGE_MSG_TEST_H_ */

