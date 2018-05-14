/*
 * QGS_ModuleMsgTest.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: mlundh
 */


#include "gtest/gtest.h"
#include "QGS_ModuleMsg.h"
#include "QGS_Module.h"
using namespace QuadGS;


TEST(QGSModuleMsgTest, StreamInOut)
{
	QGS_ModuleMsgBase::ptr msg = std::make_unique< QGS_ModuleMsgBase>(msgQuit, "Destination");
	msg->setSource("Source");

	BinaryOStream os;
	os << *msg;

	BinaryIStream is(os.get_internal_vec());

	QGS_ModuleMsgBase::ptr msgVerify = std::make_unique<QGS_ModuleMsgBase>(msgQuit);

	is >> *msgVerify;

	EXPECT_EQ(msgVerify->getType(), msg->getType());
	EXPECT_EQ(msgVerify->getDestination(), msg->getDestination());
	EXPECT_EQ(msgVerify->getSource(), msg->getSource());

}
