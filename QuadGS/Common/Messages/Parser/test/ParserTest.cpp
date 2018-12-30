/*
 * FcParserTest.cpp
 *
 *  Created on: Dec 9, 2018
 *      Author: mlundh
 */

#include "QGS_Module.h"
#include "QGS_ModuleMsg.h"
#include "Parser.h"
#include "Msg_Param.h"
#include "BinaryStream.h"
#include "gtest/gtest.h"
#include "msgAddr.h"


namespace QuadGS {


TEST(FcParser, TestParserParamMsg)
{
	Parser parser;
	Msg_Param paramMsg(FC_Log_e, 3, 0, 1, "payload");

	BinaryOStream os;

	os << paramMsg;

	QGS_ModuleMsgBase::ptr msg;

	msg = parser.parse(os.get_internal_vec());


	Msg_Param* nameMsg = static_cast<Msg_Param*>(msg.get());

	EXPECT_EQ(nameMsg->getControl(), paramMsg.getControl());
	EXPECT_EQ(nameMsg->getDestination(), paramMsg.getDestination());
	EXPECT_EQ(nameMsg->getLastinsequence(), paramMsg.getLastinsequence());
	EXPECT_EQ(nameMsg->getMsgNr(), paramMsg.getMsgNr());
	EXPECT_EQ(nameMsg->getPayload(), paramMsg.getPayload());
	EXPECT_EQ(nameMsg->getSource(), paramMsg.getSource());

}


TEST(FcParser, TestParserParamMsgFromBase)
{
	Parser parser;
	Msg_Param::ptr paramMsg = std::make_unique<Msg_Param>(FC_Log_e, 3, 0, 1, "payload");
	Msg_Param::ptr verifyMsg = std::make_unique<Msg_Param>(FC_Log_e, 3, 0, 1, "payload");

	QGS_ModuleMsgBase::ptr basePtr = std::move(paramMsg);

	BinaryOStream os;

	os << *basePtr;

	QGS_ModuleMsgBase::ptr msg;

	msg = parser.parse(os.get_internal_vec());


	Msg_Param* nameMsg = static_cast<Msg_Param*>(msg.get());

	EXPECT_EQ(nameMsg->getControl(), verifyMsg->getControl());
	EXPECT_EQ(nameMsg->getDestination(), verifyMsg->getDestination());
	EXPECT_EQ(nameMsg->getLastinsequence(), verifyMsg->getLastinsequence());
	EXPECT_EQ(nameMsg->getMsgNr(), verifyMsg->getMsgNr());
	EXPECT_EQ(nameMsg->getPayload(), verifyMsg->getPayload());
	EXPECT_EQ(nameMsg->getSource(), verifyMsg->getSource());

}

}
