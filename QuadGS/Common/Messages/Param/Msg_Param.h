#ifndef QUADGS_MESSAGE_MSG_PARAM_H_
#define QUADGS_MESSAGE_MSG_PARAM_H_
#include "QGS_ModuleMsg.h"

namespace QuadGS {

class Msg_Param: public QGS_ModuleMsg<Msg_Param>
{
public:

	Msg_Param(QGS_ModuleMsgBase& msg);

	Msg_Param(std::string destination, uint8_t control, uint8_t sequenceNr, uint8_t lastInSequence, std::string payload);

	Msg_Param(const Msg_Param& msg);

	virtual ~Msg_Param();

	virtual void dispatch(QGS_MessageHandlerBase* handler) 
	{ 
		this->dynamicDispatch(handler,this); 
	}
	
	typedef std::unique_ptr<Msg_Param> ptr;

	virtual BinaryOStream& stream(BinaryOStream& os) const;
	
	virtual BinaryIStream& stream(BinaryIStream& is);
	
		uint8_t getControl() const;

	void setControl(uint8_t control);
	uint8_t getSequencenr() const;

	void setSequencenr(uint8_t sequenceNr);
	uint8_t getLastinsequence() const;

	void setLastinsequence(uint8_t lastInSequence);
	std::string getPayload() const;

	void setPayload(std::string payload);

	
private:
	uint8_t mControl;
	uint8_t mSequencenr;
	uint8_t mLastinsequence = 0;
	std::string mPayload;
	
};
}

#endif /* QUADGS_MESSAGE_MSG_PARAM_H_ */

