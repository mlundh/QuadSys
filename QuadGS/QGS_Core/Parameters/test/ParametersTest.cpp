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

#include <memory>

#include "QGS_ParamMsg.h"
#include "QGS_MsgHeader.h"
#include "gtest/gtest.h"


using namespace QuadGS;

void defaultMsgHandler(std::shared_ptr<QGS_MsgHeader>, std::shared_ptr<QGS_Msg>)
{
    std::cout << "Got a message!" << std::endl;
}


// TODO add more tests. Use mock to set up callback handling.

class ParamTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        QuadGS::AppLog::Init("app_log", "msg_log", std::clog, severity_level::error);
        mParameters =  Parameters::create();

        std::string payload_str = "/root/tmp<5>[8]/test[3]";
        mVerifyStr = {"root<0>             \n    tmp<5>              [8]       \n        test<5>             [3]       \n"};

        QGSParamMsg::ptr payload = QGSParamMsg::Create(payload_str,0,1);

        mParameters->RegisterWriteFcn(std::bind(&defaultMsgHandler, std::placeholders::_1, std::placeholders::_2));
        QGS_MsgHeader::ptr header = QGS_MsgHeader::Create(QGS_MsgHeader::addresses::Parameters,
                QGS_MsgHeader::ParametersControl::SetTree,
                0,
                1+payload_str.length());
        mParameters->ParameterHandler(header, payload);

    }

    // virtual void TearDown() {}
    std::string mVerifyStr;

    std::shared_ptr<Parameters> mParameters;
};


TEST(Parameters, TestRegisterAndDump)
{
    QuadGS::AppLog::Init("app_log", "msg_log", std::clog, severity_level::error);
    std::shared_ptr<Parameters> mParameters =  Parameters::create();
    std::string payload_str = "/root/tmp<5>[8]/test[3]";
    QGSParamMsg::ptr payload = QGSParamMsg::Create(payload_str,0,1);

    QGS_MsgHeader::ptr header = QGS_MsgHeader::Create(QGS_MsgHeader::addresses::Parameters,
            QGS_MsgHeader::ParametersControl::SetTree,
            0,
            1+payload_str.length());

    mParameters->ParameterHandler(header, payload);
    std::string dump = mParameters->dump("");

    std::string verify_str = {"root<0>             \n    tmp<5>              [8]       \n        test<5>             [3]       \n"};

    EXPECT_EQ(verify_str, dump);
}

TEST(Parameters, TestRegisterSecondMsg)
{
    QuadGS::AppLog::Init("app_log", "msg_log", std::clog, severity_level::error);
    std::shared_ptr<Parameters> mParameters =  Parameters::create();

    // Bind dummy write function.
    mParameters->RegisterWriteFcn(std::bind(&defaultMsgHandler, std::placeholders::_1, std::placeholders::_2));

    // Create and register the first message.
    std::string payload_str = "/root/tmp<5>[8]/test[3]";
    QGSParamMsg::ptr payload = QGSParamMsg::Create(payload_str,0,0);
    QGS_MsgHeader::ptr header = QGS_MsgHeader::Create(QGS_MsgHeader::addresses::Parameters,
            QGS_MsgHeader::ParametersControl::SetTree,
            0,
            1+payload_str.length());
    mParameters->ParameterHandler(header, payload);

    // Create and register the second message.
    std::string payload_str2 = "/root/tmp<5>[8]/jus<7>/another<7>[65536]/value<7>[249037]";
    QGSParamMsg::ptr payload2 = QGSParamMsg::Create(payload_str2,1,1);
    QGS_MsgHeader::ptr header2 = QGS_MsgHeader::Create(QGS_MsgHeader::addresses::Parameters,
            QGS_MsgHeader::ParametersControl::SetTree,
            0,
            1+payload_str2.length());
    mParameters->ParameterHandler(header2, payload2);

    // Use dump to verify the registration. Remember that the ParameterHandler expects the integer representation of the fp,
    // but the dump function prints the decimal number.
    std::string dump = mParameters->dump("");
    std::string verify = "root<0>             \n    tmp<5>              [8]       \n"
            "        test<5>             [3]       \n        jus<7>              [0.000000]\n"
            "            another<7>          [1.000000]\n                value<7>            [3.800003]\n";
    EXPECT_EQ(verify, dump);
}

TEST(Parameters, TestSetAndRegister)
{
    QuadGS::AppLog::Init("app_log", "msg_log", std::clog, severity_level::error);
    std::string payload_str = "/root/tmp<5>[8]/test[3]";
    std::shared_ptr<Parameters> mParameters =  Parameters::create();

    mParameters->SetAndRegister(payload_str);
    std::string dump = mParameters->dump("");

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
    std::string dump = mParameters->dump("");
    EXPECT_EQ(mVerifyStr, dump);
}



TEST_F(ParamTest, TestInc)
{
    mParameters->add("/root/tmp 5");
    std::string result = mParameters->get("/root/tmp");

    EXPECT_EQ(result, "13");

}

TEST_F(ParamTest, TestIncTwice)
{
    mParameters->add("/root/tmp 5");
    mParameters->add("/root/tmp 1");
    std::string result = mParameters->get("/root/tmp");

    EXPECT_EQ(result, "14");

}

TEST_F(ParamTest, TestDec)
{
    mParameters->add("/root/tmp -5");
    std::string result = mParameters->get("/root/tmp");

    EXPECT_EQ(result, "3");

}
