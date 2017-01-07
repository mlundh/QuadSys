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


#include "gtest/gtest.h"
#include "QuadParamPacket.h"
#include "QCMsgHeader.h"
#include "Parameters.h"


using namespace QuadGS;

// TODO add more tests. Use mock to set up callback handling.

TEST(Parameters, TestRegisterAndDump)
{
	QuadGS::Log::Init("app_log", "msg_log", std::clog, severity_level::error);
    std::shared_ptr<Parameters> mParameters =  Parameters::create();
    std::string payload_str = "/root/tmp<5>[8]/test[3]";
    QuadParamPacket::ptr payload = QuadParamPacket::Create(payload_str,0,1);

	QCMsgHeader::ptr header = QCMsgHeader::Create(QCMsgHeader::addresses::Parameters,
			QCMsgHeader::ParametersControl::SetTree,
			0,
			1+payload_str.length());

    mParameters->ParameterHandler(header, payload);
    std::string dump = mParameters->dump("");

    const char verify[] = {'r','o','o','t','<','0','>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\n'
    		              ,' ',' ',' ',' ','t','m', 'p', '<', '5', '>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','[','8',']',' ',' ',' ',' ',' ',' ',' ','\n'
			              ,' ',' ',' ',' ',' ',' ',' ',' ','t','e','s','t','<','5','>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','[','3',']',' ',' ',' ',' ',' ',' ',' ','\n'};
    std::string verify_str(verify);

    EXPECT_EQ(verify_str, dump);
}
