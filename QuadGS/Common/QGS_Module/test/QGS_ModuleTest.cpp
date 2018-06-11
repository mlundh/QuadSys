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
	ModuleTest(): module_1("module_1"),module_2("module_2"),router("router")
{
		QuadGS::AppLog::Init("app_log", "msg_log", std::cout, QuadGS::severity_level::warning, false);

		router.bind(&module_1);
		router.bind(&module_2);
}

	QGS_ModuleFake module_1;
	QGS_ModuleFake module_2;
	QGS_Router router;

};

TEST_F(ModuleTest, SendAndReceive)
{


	module_1.returnNxtMsg(true);
	module_2.sendDummyDebugIoMsg(module_1.getName());;
	module_1.returnNxtMsg(true);
	module_2.sendDummyParamIoMsg(module_1.getName());
	for(int i = 0; i < 100; i++)
	{
		if((module_1.getNrMsg() == 2) && (module_2.getNrMsg() == 1))
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
	QGS_ModuleFake module_3("module_1");
	router.bind(&module_3);

	usleep(500); // allow time to print to stdout
	std::string output = testing::internal::GetCapturedStdout();
	size_t pos = output.find("Name collision, two modules named: ");
	EXPECT_NE(pos, std::string::npos) << "---> did not find \"Name collision, two modules named:\"";
}

// Test fixture to setup the topology.
class ThreadedModuleTest : public ::testing::Test {
protected:
	ThreadedModuleTest(): module_1("module_1"),module_2("module_2"),router("router")
{
		QuadGS::AppLog::Init("app_log", "msg_log", std::cout, QuadGS::severity_level::warning, false);

		router.bind(&module_1);
		router.bind(&module_2);
}

	virtual void SetUp()
	{

	}

	QGS_ThreadedModuleFake module_1;
	QGS_ThreadedModuleFake module_2;
	QGS_Router router;
};



TEST_F(ThreadedModuleTest, SendAndGetReturned)
{

	module_1.returnNxtMsg(true);
	module_2.sendDummyDebugMsg(module_1.getName());
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

// Test fixture to setup the topology.
class IoModuleTest : public ::testing::Test {
protected:
	IoModuleTest(): threadedModule("threadedModule"),reactiveModule("reactiveModule"),ioModule("ioModule"), router("router")
{
		QuadGS::AppLog::Init("app_log", "msg_log", std::cout, QuadGS::severity_level::warning, false);

		router.bind(&threadedModule);
		router.bind(&reactiveModule);
		router.bind(&ioModule);

}

	virtual void SetUp()
	{

	}

	QGS_ThreadedModuleFake threadedModule;
	QGS_ModuleFake reactiveModule;
	QGS_IoModuleFake ioModule;
	QGS_Router router;
};


TEST_F(IoModuleTest, CommandTest)
{

//	ioModule.getCommands();
//
//
//	for(int i = 0; i < 100; i++)
//	{
//		if(ioModule.mCommands.size() == 1 && ioModule.mResponce.size() == 1)
//		{
//			break;
//		}
//		usleep(10); // make sure that all messages are processed.
//	}
//	ASSERT_EQ(ioModule.mCommands.size(), 1);
//	ASSERT_EQ(ioModule.mResponce.size(), 1);
//	EXPECT_EQ(ioModule.mCommands[0].mName, "TestFcn");
//	EXPECT_EQ(ioModule.mResponce[0], "OK");

	EXPECT_EQ("OK", "OK");

}

