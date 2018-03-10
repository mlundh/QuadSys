/*
 * QGS_ModuleMsgTest.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: mlundh
 */


#include "gtest/gtest.h"
#include "QGS_ModuleMsg.h"
#include "QGS_CommandMsg.h"
#include "QGS_Module.h"
using namespace QuadGS;


TEST(QGSModuleMsgTest, StreamInOut)
{
	QGS_ModuleMsg::ptr msg = std::make_unique<QGS_ModuleMsg>(msgQuit);
	msg->setDestinationPort(55);
	msg->setOriginatingPort(11);
	msg->setOriginator("test");

	BinaryOStream os;
	os << *msg;

	BinaryIStream is(os.get_internal_vec());

	QGS_ModuleMsg::ptr msgVerify = std::make_unique<QGS_ModuleMsg>(msgCommand);

	is >> *msgVerify;

	EXPECT_EQ(msgVerify->getType(), msg->getType());
	EXPECT_EQ(msgVerify->getDestinationPort(), msg->getDestinationPort());
	EXPECT_EQ(msgVerify->getOriginatingPort(), msg->getOriginatingPort());
	EXPECT_EQ(msgVerify->getOriginator(), msg->getOriginator());

}

TEST(QGSCommandMsgTest, StreamInOut)
{
	QGS_CommandMsg::ptr msg = std::make_unique<QGS_CommandMsg>("first","arg","doc");
	msg->setDestinationPort(55);
	msg->setOriginatingPort(11);
	msg->setOriginator("test");

	BinaryOStream os;
	os << *msg;

	BinaryIStream is(os.get_internal_vec());
	QGS_ModuleMsg header(msgCommand);

	is >> header;
	QGS_CommandMsg::ptr msgVerify = std::make_unique<QGS_CommandMsg>(header);

	is >> *msgVerify;

	EXPECT_EQ(msgVerify->mCommand.mName, msg->mCommand.mName);
	EXPECT_EQ(msgVerify->mCommand.mArgs, msg->mCommand.mArgs);
	EXPECT_EQ(msgVerify->mCommand.mDoc, msg->mCommand.mDoc);
	EXPECT_EQ(msgVerify->getType(), msg->getType());
	EXPECT_EQ(msgVerify->getDestinationPort(), msg->getDestinationPort());
	EXPECT_EQ(msgVerify->getOriginatingPort(), msg->getOriginatingPort());
	EXPECT_EQ(msgVerify->getOriginator(), msg->getOriginator());

}

TEST(QGSCommandMsgTest, StreamInOutRsp)
{
	std::string result("result");
	QGS_CommandRsltMsg::ptr msg = std::make_unique<QGS_CommandRsltMsg>(result);
	msg->setDestinationPort(55);
	msg->setOriginatingPort(11);
	msg->setOriginator("test");

	BinaryOStream os;
	os << *msg;

	BinaryIStream is(os.get_internal_vec());
	QGS_ModuleMsg header(msgCommand);

	is >> header;
	QGS_CommandRsltMsg::ptr msgVerify = std::make_unique<QGS_CommandRsltMsg>(header);

	is >> *msgVerify;
	EXPECT_EQ(msgVerify->mResult, msg->mResult);
	EXPECT_EQ(msgVerify->getType(), msg->getType());
	EXPECT_EQ(msgVerify->getDestinationPort(), msg->getDestinationPort());
	EXPECT_EQ(msgVerify->getOriginatingPort(), msg->getOriginatingPort());
	EXPECT_EQ(msgVerify->getOriginator(), msg->getOriginator());

}

TEST(QGSCommandMsgTest, StreamInOutReq)
{
	QGS_CommandReqMsg::ptr msg = std::make_unique<QGS_CommandReqMsg>();
	msg->setDestinationPort(55);
	msg->setOriginatingPort(11);
	msg->setOriginator("test");

	BinaryOStream os;
	os << *msg;

	BinaryIStream is(os.get_internal_vec());
	QGS_ModuleMsg header(msgCommand);

	is >> header;
	QGS_CommandReqMsg::ptr msgVerify = std::make_unique<QGS_CommandReqMsg>(header);

	is >> *msgVerify;
	EXPECT_EQ(msgVerify->getType(), msg->getType());
	EXPECT_EQ(msgVerify->getDestinationPort(), msg->getDestinationPort());
	EXPECT_EQ(msgVerify->getOriginatingPort(), msg->getOriginatingPort());
	EXPECT_EQ(msgVerify->getOriginator(), msg->getOriginator());

}

TEST(QGSCommandMsgTest, StreamInOutReqRsp)
{
	std::vector<QGS_UiCommand_> mCommands;
	mCommands.push_back(QGS_UiCommand_("name", NULL, "doc"));
	mCommands.push_back(QGS_UiCommand_("name1", NULL, "doc1"));

	QGS_CommandReqRspMsg::ptr msg = std::make_unique<QGS_CommandReqRspMsg>(mCommands);

	msg->setDestinationPort(55);
	msg->setOriginatingPort(11);
	msg->setOriginator("test");

	BinaryOStream os;
	os << *msg;

	BinaryIStream is(os.get_internal_vec());
	QGS_ModuleMsg header(msgCommand);

	is >> header;
	QGS_CommandReqRspMsg::ptr msgVerify = std::make_unique<QGS_CommandReqRspMsg>(header);

	is >> *msgVerify;
	EXPECT_EQ(msgVerify->mCommands[0].mName, msg->mCommands[0].mName);
	EXPECT_EQ(msgVerify->mCommands[0].mArgs, msg->mCommands[0].mArgs);
	EXPECT_EQ(msgVerify->mCommands[0].mDoc,  msg->mCommands[0].mDoc);
	EXPECT_EQ(msgVerify->mCommands[1].mName, msg->mCommands[1].mName);
	EXPECT_EQ(msgVerify->mCommands[1].mArgs, msg->mCommands[1].mArgs);
	EXPECT_EQ(msgVerify->mCommands[1].mDoc,  msg->mCommands[1].mDoc);
	EXPECT_EQ(msgVerify->getType(), msg->getType());
	EXPECT_EQ(msgVerify->getDestinationPort(), msg->getDestinationPort());
	EXPECT_EQ(msgVerify->getOriginatingPort(), msg->getOriginatingPort());
	EXPECT_EQ(msgVerify->getOriginator(), msg->getOriginator());

}
