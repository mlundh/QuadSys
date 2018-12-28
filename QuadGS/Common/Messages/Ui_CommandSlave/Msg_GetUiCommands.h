#ifndef QUADGS_MESSAGE_MSG_GETUICOMMANDS_H_
#define QUADGS_MESSAGE_MSG_GETUICOMMANDS_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_GetUiCommands: public QGS_ModuleMsg<Msg_GetUiCommands>
{
public:

	Msg_GetUiCommands(QGS_ModuleMsgBase& msg);

	Msg_GetUiCommands(const std::string& destination);

	Msg_GetUiCommands(const Msg_GetUiCommands& msg);

	virtual ~Msg_GetUiCommands();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_GetUiCommands> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	
	
private:
	
};
}

#endif /* QUADGS_MESSAGE_MSG_GETUICOMMANDS_H_ */

