/*
* AppLogTest.cpp
*
* Copyright (C) 2021  Martin Lundh
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
#include "AppLog.h"

#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "Msg_GsAppLog.h"

using namespace QuadGS;

class AppLogTest : public ::testing::Test
{
public:
    AppLog log;
    void sendFcn(std::unique_ptr<QGS_ModuleMsgBase> message)
    {
        // We should only receive Msg_GsAppLog messages, so we can cast to that type.
        Msg_GsAppLog *tmp = dynamic_cast<Msg_GsAppLog *>(message.get());
        if (tmp != nullptr)
        {
            message.release();
            mMessage.reset(tmp);
        }
    }
    AppLogTest() : log("Testing", std::bind(&AppLogTest::sendFcn, this, std::placeholders::_1))
    {
    }
    std::unique_ptr<Msg_GsAppLog> mMessage;

protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(AppLogTest, TestStreamOperator)
{
    log.setModuleLogLevel(debug);

    log.setMsgLogLevel(debug);
    log << "Stuff! " << 5.4 << " test";
    log.flush();

    std::string expected("[debug] [Testing]: Stuff! 5.4 test\n");
    ASSERT_TRUE(mMessage);
    std::string message(mMessage->getMessage());
    std::string msgNoTime = message.substr(15);

    EXPECT_EQ(expected, msgNoTime);
}

TEST_F(AppLogTest, TestDefaultLL)
{
    log.setModuleLogLevel(info);

    log << "Stuff! " << 5.4 << " test";
    log.flush();

    std::string expected("[info] [Testing]: Stuff! 5.4 test\n");
    ASSERT_TRUE(mMessage);
    std::string message(mMessage->getMessage());
    std::string msgNoTime = message.substr(15);

    EXPECT_EQ(expected, msgNoTime);
}

TEST_F(AppLogTest, TestTwo)
{
    log.setModuleLogLevel(debug);
    {
        log << "Stuff! " << 5 << " test";
        log.flush();

        std::string expected("[info] [Testing]: Stuff! 5 test\n");
        ASSERT_TRUE(mMessage);
        std::string message(mMessage->getMessage());
        std::string msgNoTime = message.substr(15);
        //std::cout << mMessage->getMessage();
        EXPECT_EQ(expected, msgNoTime);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(6));
    {
        log.setMsgLogLevel(debug);

        log << "Hello! " << 5 << " test";
        log.flush();

        std::string expected("[debug] [Testing]: Hello! 5 test\n");
        ASSERT_TRUE(mMessage);

        std::string message(mMessage->getMessage());
        std::string msgNoTime = message.substr(15);
        //std::cout << mMessage->getMessage();

        EXPECT_EQ(expected, msgNoTime);
    }
}


TEST_F(AppLogTest, TestLogLevel)
{
    log.setModuleLogLevel(info);
    {
        // We should not add anything the the logger as the module loglevel is lower than the messageLogLevel.
        log.setMsgLogLevel(debug);
        log << "Stuff! " << 5 << " test";
        log.flush();

        std::string expected("[debug] [Testing]: Stuff! 5 test\n");
        // No log should have been created as the log level of the message is higher than that of the module.
        ASSERT_FALSE(mMessage); 
    }
    {
        log.setMsgLogLevel(info);

        log << "Hello! " << 5 << " test";
        log.flush();

        std::string expected("[info] [Testing]: Hello! 5 test\n");
        // Here the log should have been created as the msg log level is equal to the module log level.
        ASSERT_TRUE(mMessage); 

        std::string message(mMessage->getMessage());
        std::string msgNoTime = message.substr(15);

        EXPECT_EQ(expected, msgNoTime);
    }
}
TEST_F(AppLogTest, TestMacro)
{
    log.setModuleLogLevel(debug);

    LOG_DEBUG(log, "Stuff! " << 5.4 << " test");


    std::string expected("[debug] [Testing]: Stuff! 5.4 test\n");
    ASSERT_TRUE(mMessage);
    std::string message(mMessage->getMessage());
    std::string msgNoTime = message.substr(15);

    EXPECT_EQ(expected, msgNoTime);
}