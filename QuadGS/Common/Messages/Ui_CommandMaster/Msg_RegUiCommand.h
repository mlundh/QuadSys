#ifndef QUADGS_MESSAGE_MSG_REGUICOMMAND_H_
#define QUADGS_MESSAGE_MSG_REGUICOMMAND_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_RegUiCommand: public QGS_ModuleMsg<Msg_RegUiCommand>
{
public:

	Msg_RegUiCommand(QGS_ModuleMsgBase& msg);

	Msg_RegUiCommand(std::string destination, std::string command, std::string doc);

	Msg_RegUiCommand(const Msg_RegUiCommand& msg);

	virtual ~Msg_RegUiCommand();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_RegUiCommand> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
		std::string getCommand() const;

	void setCommand(std::string command);
	std::string getDoc() const;

	void setDoc(std::string doc);

	
private:
	std::string mCommand;
	std::string mDoc;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_REGUICOMMAND_H_ */

