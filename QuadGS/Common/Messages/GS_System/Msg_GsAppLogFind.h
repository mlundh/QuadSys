#ifndef QUADGS_MESSAGE_MSG_GSAPPLOGFIND_H_
#define QUADGS_MESSAGE_MSG_GSAPPLOGFIND_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_GsAppLogFind: public QGS_ModuleMsg<Msg_GsAppLogFind>
{
public:

	Msg_GsAppLogFind(QGS_ModuleMsgBase& msg);

	Msg_GsAppLogFind(const msgAddr_t destination, uint8_t command, std::string toFind, std::string found);

	Msg_GsAppLogFind(const Msg_GsAppLogFind& msg);

	virtual ~Msg_GsAppLogFind();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_GsAppLogFind> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint8_t getCommand() const;

	void setCommand(uint8_t command);
	
	std::string getTofind() const;

	void setTofind(std::string toFind);
	
	std::string getFound() const;

	void setFound(std::string found);
	

	
private:
	uint8_t mCommand;
	std::string mTofind;
	std::string mFound;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_GSAPPLOGFIND_H_ */

