#ifndef QUADGS_MESSAGE_MSG_FINDPARAM_H_
#define QUADGS_MESSAGE_MSG_FINDPARAM_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_FindParam: public QGS_ModuleMsg<Msg_FindParam>
{
public:

	Msg_FindParam(QGS_ModuleMsgBase& msg);

	Msg_FindParam(std::string destination, std::string toFind, std::string found);

	Msg_FindParam(const Msg_FindParam& msg);

	virtual ~Msg_FindParam();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_FindParam> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
		std::string getTofind() const;

	void setTofind(std::string toFind);
	std::string getFound() const;

	void setFound(std::string found);

	
private:
	std::string mTofind;
	std::string mFound;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_FINDPARAM_H_ */

