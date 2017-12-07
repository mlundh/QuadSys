/*
 * MockCore.h
 *
 *  Created on: 30 May 2017
 *      Author: mlundh
 */

#ifndef QUADGS_QGS_CORE_TEST_MOCKCORE_H_
#define QUADGS_QGS_CORE_TEST_MOCKCORE_H_
#include "../../QGS_Core/Core.h"
#include "gmock/gmock.h"  // Brings in Google Mock.

namespace QuadGS {

class MockCore: public Core {
public:
	MockCore();
	virtual ~MockCore();
	MOCK_METHOD1(bind, void(IoBase* IoPtr));
	MOCK_METHOD1(bind, void(UiBase* UiPtr));
	MOCK_METHOD0(getCommands, std::vector<Command::ptr> ());
	MOCK_METHOD2(write, void ( std::shared_ptr<QCMsgHeader> header, std::shared_ptr<QuadGSMsg> payload));
};







} /* namespace QuadGS */

#endif /* QUADGS_QGS_CORE_TEST_MOCKCORE_H_ */
