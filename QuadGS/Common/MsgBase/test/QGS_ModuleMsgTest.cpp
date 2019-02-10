/*
 * QGS_ModuleMsgTest.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: mlundh
 */


#include "gtest/gtest.h"
#include "QGS_ModuleMsg.h"
#include "Msg_Stop.h"

using namespace QuadGS;


TEST(QGSModuleMsgTest, StreamInOut)
{

	QGS_ModuleMsgBase::ptr msg = std::make_unique< Msg_Stop>(msgAddr_t::GS_Dbg_e);
	msg->setSource(msgAddr_t::FC_Param_e);

	BinaryOStream os;
	os << *msg;

	BinaryIStream is(os.get_internal_vec());

	Msg_Stop::ptr msgVerify = std::make_unique<Msg_Stop>(msgAddr_t::GS_Dbg_e);

	is >> *msgVerify;

	EXPECT_EQ(msgVerify->getType(), msg->getType());
	EXPECT_EQ(msgVerify->getDestination(), msg->getDestination());
	EXPECT_EQ(msgVerify->getSource(), msg->getSource());

}
