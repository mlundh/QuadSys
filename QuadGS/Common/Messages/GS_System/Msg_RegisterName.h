#ifndef QUADGS_MESSAGE_MSG_REGISTERNAME_H_
#define QUADGS_MESSAGE_MSG_REGISTERNAME_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_RegisterName: public QGS_ModuleMsg<Msg_RegisterName>
{
public:

	Msg_RegisterName(QGS_ModuleMsgBase& msg);

	Msg_RegisterName(const std::string& destination, std::string name);

	Msg_RegisterName(const Msg_RegisterName& msg);

	virtual ~Msg_RegisterName();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_RegisterName> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
		std::string getName() const;

	void setName(std::string name);

	
private:
	std::string mName;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_REGISTERNAME_H_ */

