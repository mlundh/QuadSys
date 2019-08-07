/*
 * ParametersTest.cpp
 *
 * Copyright (C) 2017 Martin Lundh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Parameters.h"
#include "FakeModule.h"
#include "QGS_Router.h"

#include <memory>
#include <iostream>

#include "msg_enums.h"
#include "gtest/gtest.h"


using namespace QuadGS;

// TODO add more tests. Use mock to set up callback handling.

class ParamTest : public ::testing::Test {
protected:
	ParamTest():mParameters(msgAddr_t::GS_Param_e), mFake(msgAddr_t::GS_Dbg_e), router(msgAddr_t::GS_Router_e) {

		QuadGS::AppLog::Init("app_log", "msg_log", std::clog, severity_level::error, false);

		std::string payload_str = "/root/tmp<5>[8]/test[3]";
		mParameters.SetAndRegister(payload_str);
		mVerifyStr = {"root<0>             \n    tmp<5>              [8]       \n        test<5>             [3]       \n"};


		router.bind(&mParameters);
		router.bind(&mFake);
	}

	// virtual void TearDown() {}
	std::string mVerifyStr;
	Parameters mParameters;
	FakeModuleParam mFake;
	QGS_Router router;

};


TEST(Parameters, TestRegisterAndDump)
{
	QuadGS::AppLog::Init("app_log", "msg_log", std::clog, severity_level::error, false);
	Parameters mParameters(msgAddr_t::GS_Dbg_e);
	std::string payload_str = "/root/tmp<5>[8]/test[3]";


	mParameters.SetAndRegister(payload_str);

	std::string dump = mParameters.dump("");

	std::string verify_str = {"root<0>             \n    tmp<5>              [8]       \n        test<5>             [3]       \n"};

	EXPECT_EQ(verify_str, dump);
}

TEST(Parameters, TestRegisterSecondMsg)
{
	QuadGS::AppLog::Init("app_log", "msg_log", std::clog, severity_level::error, false);

	// Create and register the first message.
	std::string payload_str = "/root/tmp<5>[8]/test[3]";
	Parameters mParameters(msgAddr_t::GS_Dbg_e);

	mParameters.SetAndRegister(payload_str);

	// Create and register the second message.
	std::string payload_str2 = "/root/tmp<5>[8]/jus<7>/another<7>[65536]/value<7>[249037]";

	mParameters.SetAndRegister(payload_str2);

	// Use dump to verify the registration. Remember that the ParameterHandler expects the integer representation of the fp,
	// but the dump function prints the decimal number.
	std::string dump = mParameters.dump("");
	std::string verify = "root<0>             \n    tmp<5>              [8]       \n"
			"        test<5>             [3]       \n        jus<7>              [0.000000]\n"
			"            another<7>          [1.000000]\n                value<7>            [3.800003]\n";
	EXPECT_EQ(verify, dump);
}

TEST(Parameters, TestSetAndRegister)
{
	QuadGS::AppLog::Init("app_log", "msg_log", std::clog, severity_level::error, false);
	std::string payload_str = "/root/tmp<5>[8]/test[3]";
	Parameters mParameters(msgAddr_t::GS_Dbg_e);

	mParameters.SetAndRegister(payload_str);
	std::string dump = mParameters.dump("");

	std::string verify_str = {"root<0>             \n    tmp<5>              [8]       \n        test<5>             [3]       \n"};

	EXPECT_EQ(verify_str, dump);
}

TEST(ParamValue, TestFpStringToValue)
{
	QGS_TreeValue TreeValue(QGS_TreeValue::fp_16_16_variable_type);

	int32_t value = TreeValue.StringToIntFixed("5.5");

	EXPECT_EQ(DOUBLE_TO_FIXED(5.5, MAX16f), value);

	value = TreeValue.StringToIntFixed("-5.5");

	EXPECT_EQ(DOUBLE_TO_FIXED(-5.5, MAX16f), value);

}

TEST(ParamValue, TestModifyValueFp)
{
	QGS_TreeValue TreeValue(QGS_TreeValue::fp_16_16_variable_type);

	TreeValue.SetValue("5.5");

	TreeValue.ModifyValue("-5.5");

	EXPECT_EQ("0.000015", TreeValue.GetValue(true));
}

TEST_F(ParamTest, TestSetup)
{
	std::string dump = mParameters.dump("");
	EXPECT_EQ(mVerifyStr, dump);
}


TEST_F(ParamTest, TestMessage)
{
	std::string payload_str = "/root/tmp/test[555]";
	mFake.sendDummyParamMsg(ParamCtrl::param_set, payload_str);
	mFake.waitForMsg();

	std::string result = mParameters.get("/root/tmp/test");
	EXPECT_EQ("555", result);
}

TEST_F(ParamTest, TestGetCommands)
{
	mFake.sendGetCommands();
	mFake.waitForMsg();
	int i = 0;
	while((mFake.mCommands.size() < mParameters.mCommands.size()) && (i < 500))
	{
		mFake.waitForMsg();
		i++;
	}

	EXPECT_EQ(mFake.mCommands.size(), mParameters.mCommands.size());

	for(size_t i = 0; i < mFake.mCommands.size(); i++)
	{
		EXPECT_EQ(mFake.mCommands[i].first, mParameters.mCommands[i].command);
		EXPECT_EQ(mFake.mCommands[i].second, mParameters.mCommands[i].doc);

	}
}


TEST_F(ParamTest, TestFireCommand)
{
	mFake.sendFireCommand("paramAdd","/root/tmp/test 1");
	mFake.waitForMsg();
	if(mFake.getNrResult() != 1)
	{
		mFake.waitForMsg();
	}
	ASSERT_EQ(mFake.getNrResult(), 1);
	std::string result = mFake.getLastResult();
	EXPECT_EQ("4", result);
}

TEST_F(ParamTest, TestFireCommand2)
{
	mFake.sendFireCommand("paramDumpTree","");
	mFake.waitForMsg();

	ASSERT_EQ(mFake.getNrResult(), 1);
	std::string result = mFake.getLastResult();
	EXPECT_EQ(mVerifyStr, result);
}

TEST_F(ParamTest, TestInc)
{
	mParameters.add("/root/tmp 5");
	std::string result = mParameters.get("/root/tmp");

	EXPECT_EQ(result, "13");

}

TEST_F(ParamTest, TestIncTwice)
{
	mParameters.add("/root/tmp 5");
	mParameters.add("/root/tmp 1");
	std::string result = mParameters.get("/root/tmp");

	EXPECT_EQ(result, "14");

}

TEST_F(ParamTest, TestDec)
{
	mParameters.add("/root/tmp -5");
	std::string result = mParameters.get("/root/tmp");

	EXPECT_EQ(result, "3");

}
