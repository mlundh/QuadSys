/*
 * LogHandlerTest.cpp
 *
 *  Created on: Aug 10, 2018
 *      Author: mlundh
 */



#include "LogHandler.h"
#include "FakeLogModule.h"
#include "QGS_Router.h"

#include <memory>

#include "msg_enums.h"
#include "gtest/gtest.h"

using namespace QuadGS;



class LogHandlerTest : public ::testing::Test {
protected:
	LogHandlerTest():mLog(msgAddr_t::GS_Log_e,msgAddr_t::FC_Log_e), mFake(msgAddr_t::GS_Dbg_e), router(msgAddr_t::GS_Router_e) {

		router.bind(&mLog);
		router.bind(&mFake);
	}

	// virtual void TearDown() {}
	LogHandler mLog;
	FakeLogModule mFake;
	QGS_Router router;
};



TEST_F(LogHandlerTest, TestGetCommands)
{
	mFake.sendGetCommands(msgAddr_t::GS_Log_e);
	mFake.waitForMsg();
	int timeout = 100;
	while((mFake.mCommands.size() < mLog.mCommands.size()) && (timeout > 0))
	{
		mFake.waitForMsg();
		timeout--;
	}

	EXPECT_EQ(mFake.mCommands.size(), mLog.mCommands.size());

	for(size_t i = 0; i < mFake.mCommands.size(); i++)
	{
		EXPECT_EQ(mFake.mCommands[i].first, mLog.mCommands[i].command);
		EXPECT_EQ(mFake.mCommands[i].second, mLog.mCommands[i].doc);

	}
}


TEST_F(LogHandlerTest, TestPrintNames)
{
	Msg_Log msg(msgAddr_t::GS_Log_e, LogCtrl_t::Name, "/test<2>[5]/test2<1>[6]");
	std::string verify = "test        5           \ntest2       6           \n";
	// Do not do this in real code!This is only for testing. Real code should
	// rely on messages from FC to populate the mapping.
	mLog.process(&msg);

	mFake.sendFireCommand(msgAddr_t::GS_Log_e, "logPrintNameMapping","");
	mFake.waitForMsg();
	if(mFake.getNrResult() != 1)
	{
		mFake.waitForMsg();
	}
	ASSERT_EQ(mFake.getNrResult(), 1);
	std::string result = mFake.getLastResult();
	EXPECT_EQ(verify, result);
}

TEST_F(LogHandlerTest, TestGetEntry)
{
	Msg_Log msg(msgAddr_t::GS_Log_e, LogCtrl_t::Name, "/test<2>[5]/test2<1>[6]");
	// Do not do this in real code!This is only for testing. Real code should
	// rely on messages from FC to populate the mapping.
	mLog.process(&msg);

	Msg_Log msgEntry(msgAddr_t::GS_Log_e, LogCtrl_t::Entry, "[6][11111][7]/[5][11111][8]");
	mLog.process(&msgEntry);

    std::ifstream LogFile("LogFile.txt");

    std::string line;
    int a, b, c, d, e, f;
    char delimiter;

    LogFile >> a >> delimiter >> b >> delimiter >> c;

    LogFile >> d >> delimiter >> e >> delimiter >> f;

	ASSERT_EQ(a, 6);
	ASSERT_EQ(b, 11111);
	ASSERT_EQ(c, 7);

	ASSERT_EQ(d, 5);
	ASSERT_EQ(e, 11111);
	ASSERT_EQ(f, 8);
}
