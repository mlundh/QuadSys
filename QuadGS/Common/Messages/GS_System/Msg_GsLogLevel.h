#ifndef QUADGS_MESSAGE_MSG_GSLOGLEVEL_H_
#define QUADGS_MESSAGE_MSG_GSLOGLEVEL_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_GsLogLevel: public QGS_ModuleMsg<Msg_GsLogLevel>
{
public:

	Msg_GsLogLevel(QGS_ModuleMsgBase& msg);

	Msg_GsLogLevel(const msgAddr_t destination, uint8_t command, uint32_t logLevel);

	Msg_GsLogLevel(const Msg_GsLogLevel& msg);

	virtual ~Msg_GsLogLevel();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_GsLogLevel> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
	uint8_t getCommand() const;

	void setCommand(uint8_t command);
	
	uint32_t getLoglevel() const;

	void setLoglevel(uint32_t logLevel);
	

	
private:
	uint8_t mCommand;
	uint32_t mLoglevel;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_GSLOGLEVEL_H_ */

