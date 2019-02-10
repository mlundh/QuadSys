#ifndef QUADGS_MESSAGE_MSG_ERROR_H_
#define QUADGS_MESSAGE_MSG_ERROR_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_Error: public QGS_ModuleMsg<Msg_Error>
{
public:

	Msg_Error(QGS_ModuleMsgBase& msg);

	Msg_Error(const msgAddr_t destination, std::string error);

	Msg_Error(const Msg_Error& msg);

	virtual ~Msg_Error();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Error> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	std::string getError() const;

	void setError(std::string error);
	

	
private:
	std::string mError;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_ERROR_H_ */

