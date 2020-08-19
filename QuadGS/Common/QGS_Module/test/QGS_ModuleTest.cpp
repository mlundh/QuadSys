/*
 * QGS_ModuleTest.cpp
 *
 *  Copyright (C) 2018 Martin Lundh
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





#include "gtest/gtest.h"
#include "QGS_ModuleFake.h"
#include "QGS_Router.h"

using namespace QuadGS;

// Test fixture to setup the topology.
class ModuleTest : public ::testing::Test {
protected:
	ModuleTest(): module_1(msgAddr_t::GS_SetpointGen_e),module_2(msgAddr_t::GS_Log_e),router(msgAddr_t::GS_Router_e)
{
		QuadGS::AppLog::Init("app_log", "msg_log", std::cout, QuadGS::severity_level::warning);

		router.bind(&module_1);
		router.bind(&module_2);
}

	FakeModule module_1;
	FakeModule module_2;
	QGS_Router router;

};

TEST_F(ModuleTest, SendAndReceive)
{


	module_1.returnNxtMsg(true);
	module_2.sendDummyDebugMsg(module_1.getName());;
	module_1.returnNxtMsg(true);
	module_2.sendDummyParamMsg(module_1.getName());
	for(int i = 0; i < 100; i++)
	{
		if((module_1.getNrMsg() == 2) && (module_2.getNrMsg() == 2))
		{
			break;
		}
		usleep(500); // make sure that all messages are processed.
	}
	EXPECT_EQ(module_1.getNrMsg(), 2);
	EXPECT_EQ(module_2.getNrMsg(), 2);

}

TEST_F(ModuleTest, TwoSameName)
{

	testing::internal::CaptureStdout();
	FakeModule module_3(msgAddr_t::GS_SetpointGen_e);
	router.bind(&module_3);

	usleep(500); // allow time to print to stdout
	std::string output = testing::internal::GetCapturedStdout();
	size_t pos = output.find("Name collision, two modules named: ");
	EXPECT_NE(pos, std::string::npos) << "---> did not find \"Name collision, two modules named:\"";
}

// Test fixture to setup the topology.
class ThreadedModuleTest : public ::testing::Test {
protected:
	ThreadedModuleTest(): module_1(msgAddr_t::GS_SetpointGen_e),module_2(msgAddr_t::GS_Log_e),module_3(msgAddr_t::GS_Param_e),router(msgAddr_t::GS_Router_e)
{
		QuadGS::AppLog::Init("app_log", "msg_log", std::cout, QuadGS::severity_level::warning);

		router.bind(&module_1);
		router.bind(&module_2);
		router.bind(&module_3);
}

	virtual void SetUp()
	{

	}

	QGS_ThreadedModuleFake module_1;
	QGS_ThreadedModuleFake module_2;
	QGS_ThreadedModuleFake module_3;
	QGS_Router router;
};



TEST_F(ThreadedModuleTest, SendAndGetReturned)
{

	module_1.returnNxtMsg(true);
	module_2.sendDummyLogMsg(module_1.getName());
	module_1.returnNxtMsg(true);
	module_2.sendDummyParamMsg(module_1.getName());

	for(int i = 0; i < 100; i++)
	{
		if((module_1.getNrMsg() == 2) && (module_2.getNrMsg() == 1))
		{
			break;
		}
		usleep(5); // make sure that all messages are processed.
	}
	EXPECT_EQ(module_1.getNrMsg(), 2);
	EXPECT_EQ(module_2.getNrMsg(), 1); // only param msg should be returned.
}



TEST_F(ThreadedModuleTest, SendABunch)
{

	module_2.sendDummyLogMsg(module_1.getName());
	module_2.sendDummyParamMsg(module_1.getName());
	module_2.sendDummyLogMsg(module_1.getName());
	module_2.sendDummyLogMsg(module_1.getName());

	module_1.sendDummyLogMsg(module_2.getName());
	module_1.sendDummyLogMsg(module_2.getName());
	module_1.sendDummyLogMsg(module_2.getName());
	module_1.sendDummyLogMsg(module_2.getName());

	module_1.sendDummyLogMsg(module_3.getName());
	module_1.sendDummyLogMsg(module_3.getName());
	module_1.sendDummyLogMsg(module_3.getName());
	module_1.sendDummyLogMsg(module_3.getName());

	module_3.sendDummyLogMsg(module_1.getName());
	module_3.sendDummyLogMsg(module_1.getName());
	module_3.sendDummyLogMsg(module_1.getName());
	module_3.sendDummyLogMsg(module_1.getName());

	for(int i = 0; i < 100; i++)
	{
		if((module_1.getNrMsg() == 8) && (module_2.getNrMsg() == 4))
		{
			break;
		}
		usleep(5); // make sure that all messages are processed.
	}
	EXPECT_EQ(module_1.getNrMsg(), 8);
	EXPECT_EQ(module_2.getNrMsg(), 4);
	EXPECT_EQ(module_3.getNrMsg(), 4);

}


// Test fixture to setup the topology.
class MultipleModuleTypesTest : public ::testing::Test {
protected:
	MultipleModuleTypesTest(): tmodule_1(msgAddr_t::Unassigned_e),tmodule_2(msgAddr_t::GS_Log_e),
	tmodule_3(msgAddr_t::GS_Param_e),rmodule_1(msgAddr_t::GS_SetpointGen_e),rmodule_2(msgAddr_t::GS_GUI_e),
	router(msgAddr_t::GS_Router_e)
{
		QuadGS::AppLog::Init("app_log", "msg_log", std::cout, QuadGS::severity_level::warning);

		router.bind(&tmodule_1);
		router.bind(&tmodule_2);
		router.bind(&tmodule_3);
		router.bind(&rmodule_1);
		router.bind(&rmodule_2);

}

	virtual void SetUp()
	{

	}

	QGS_ThreadedModuleFake tmodule_1;
	QGS_ThreadedModuleFake tmodule_2;
	QGS_ThreadedModuleFake tmodule_3;
	FakeModule rmodule_1;
	FakeModule rmodule_2;



	QGS_Router router;
};


TEST_F(MultipleModuleTypesTest, SendABunch)
{

	tmodule_2.sendDummyLogMsg(rmodule_1.getName());
	tmodule_2.sendDummyParamMsg(rmodule_1.getName());
	tmodule_2.sendDummyLogMsg(rmodule_1.getName());
	tmodule_2.sendDummyLogMsg(rmodule_1.getName());

	rmodule_1.sendDummyDebugMsg(tmodule_2.getName());
	rmodule_1.sendDummyDebugMsg(tmodule_2.getName());
	rmodule_1.sendDummyDebugMsg(tmodule_2.getName());
	rmodule_1.sendDummyDebugMsg(tmodule_2.getName());

	rmodule_1.sendDummyDebugMsg(tmodule_3.getName());
	rmodule_1.sendDummyDebugMsg(tmodule_3.getName());
	rmodule_1.sendDummyDebugMsg(tmodule_3.getName());
	rmodule_1.sendDummyDebugMsg(tmodule_3.getName());


	for(int i = 0; i < 100; i++)
	{
		if((rmodule_1.getNrMsg() == 4) && (tmodule_2.getNrMsg() == 4) && (tmodule_3.getNrMsg() == 4))
		{
			break;
		}
		usleep(5); // make sure that all messages are processed.
	}
	EXPECT_EQ(rmodule_1.getNrMsg(), 4);
	EXPECT_EQ(tmodule_2.getNrMsg(), 4);
	EXPECT_EQ(tmodule_3.getNrMsg(), 4);

}
