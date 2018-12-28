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
	Msg_Param paramMsg(msgAddrStr[FC_Log_e], 3, 0, 1, "payload");

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

}
