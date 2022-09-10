#ifndef QUADGS_MESSAGE_MSG_INITEXTERNAL_H_
#define QUADGS_MESSAGE_MSG_INITEXTERNAL_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_InitExternal: public QGS_ModuleMsg<Msg_InitExternal>
{
public:

	Msg_InitExternal(QGS_ModuleMsgBase& msg);

	Msg_InitExternal(const msgAddr_t destination, std::string init);

	Msg_InitExternal(const Msg_InitExternal& msg);

	virtual ~Msg_InitExternal();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_InitExternal> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::string getInit() const;

	void setInit(std::string init);
	

	
private:
	std::string mInit;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_INITEXTERNAL_H_ */

