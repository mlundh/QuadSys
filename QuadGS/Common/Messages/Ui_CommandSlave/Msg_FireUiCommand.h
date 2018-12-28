#ifndef QUADGS_MESSAGE_MSG_FIREUICOMMAND_H_
#define QUADGS_MESSAGE_MSG_FIREUICOMMAND_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_FireUiCommand: public QGS_ModuleMsg<Msg_FireUiCommand>
{
public:

	Msg_FireUiCommand(QGS_ModuleMsgBase& msg);

	Msg_FireUiCommand(const msgAddr_t destination, std::string command, std::string args);

	Msg_FireUiCommand(const Msg_FireUiCommand& msg);

	virtual ~Msg_FireUiCommand();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_FireUiCommand> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
		std::string getCommand() const;

	void setCommand(std::string command);
	std::string getArgs() const;

	void setArgs(std::string args);

	
private:
	std::string mCommand;
	std::string mArgs;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_FIREUICOMMAND_H_ */

