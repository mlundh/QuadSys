/*
 * MockCore.h
 *
 *  Created on: 30 May 2017
 *      Author: mlundh
 */

#ifndef QUADGS_CORE_TEST_MOCKQGSCOREINTERFACE_H_
#define QUADGS_CORE_TEST_MOCKQGSCOREINTERFACE_H_
#include "QGS_CoreInterface.h"
#include "gmock/gmock.h"  // Brings in Google Mock.

namespace QuadGS {

class MockCore: public QGS_CoreInterface {
public:
	MockCore();
	virtual ~MockCore();
	MOCK_METHOD1(bind, void(QGS_IoInterface* IoPtr));
	MOCK_METHOD1(bind, void(QGS_UiInterface* UiPtr));
	MOCK_METHOD0(getCommands, std::vector<QGS_UiCommand::ptr> ());
	MOCK_METHOD2(write, void ( std::shared_ptr<QGS_IoHeader> header, std::shared_ptr<QGS_Msg> payload));
};







} /* namespace QuadGS */

#endif /* QUADGS_CORE_TEST_MOCKQGSCOREINTERFACE_H_ */
