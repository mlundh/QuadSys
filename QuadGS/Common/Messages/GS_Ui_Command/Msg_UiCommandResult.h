#ifndef QUADGS_MESSAGE_MSG_UICOMMANDRESULT_H_
#define QUADGS_MESSAGE_MSG_UICOMMANDRESULT_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_UiCommandResult: public QGS_ModuleMsg<Msg_UiCommandResult>
{
public:

	Msg_UiCommandResult(QGS_ModuleMsgBase& msg);

	Msg_UiCommandResult(const msgAddr_t destination, std::string result);

	Msg_UiCommandResult(const Msg_UiCommandResult& msg);

	virtual ~Msg_UiCommandResult();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_UiCommandResult> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::string getResult() const;

	void setResult(std::string result);
	

	
private:
	std::string mResult;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_UICOMMANDRESULT_H_ */

