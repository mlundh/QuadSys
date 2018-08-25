/*
 * FakeLogModule.h
 *
 *  Created on: Aug 10, 2018
 *      Author: mlundh
 */

#ifndef QUADGS_MODULES_LOGHANDLER_TEST_FAKELOGMODULE_H_
#define QUADGS_MODULES_LOGHANDLER_TEST_FAKELOGMODULE_H_

#include "FakeUiComModuleBase.h"
#include "Msg_Log.h"
namespace QuadGS {
/**
 * Fake class that acts both as a UI module and as a Log moduel to test the
 * log module.
 */
class FakeLogModule: public FakeUiComModuleBase
					, public QGS_MessageHandler<Msg_GetUiCommands>
					, public QGS_MessageHandler<Msg_FireUiCommand>
					, public QGS_MessageHandler<Msg_Log>
{
public:
	FakeLogModule(std::string name):
		QGS_MessageHandlerBase(name),FakeUiComModuleBase(name)
	{

	}
	virtual ~FakeLogModule(){};


	virtual void process(Msg_GetUiCommands* message)
	{
		setNewMsg();
	}
	virtual void process(Msg_FireUiCommand* message)
	{
		setNewMsg();
	}
	virtual void process(Msg_Log* message)
	{
		setNewMsg();
	}


};

} /* namespace QuadGS */

#endif /* QUADGS_MODULES_LOGHANDLER_TEST_FAKELOGMODULE_H_ */
