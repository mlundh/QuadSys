/*
 * SlipPacketTest.cpp
 *
 *  Copyright (C) 2017 Martin Lundh
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


#include "SlipPacket.h"

#include <vector>

#include "gtest/gtest.h"
#include "BinaryStream.h"
#include "Msg_Param.h"

using namespace QuadGS;

TEST(SlipPacketTest, TestPayload)
{
	std::vector<unsigned char> test = {'t','e','s','t','i','n','g'};
	SlipPacket::ptr encode = SlipPacket::Create(test, true);

	SlipPacket::ptr decode = SlipPacket::Create(encode->GetPacket(), false);

    EXPECT_EQ(encode->GetPayload(), decode->GetPayload());
}


TEST(SlipPacketTest, TestPacket)
{
	std::vector<unsigned char> test = {'t','e','s','t','i','n','g'};
	SlipPacket::ptr encode = SlipPacket::Create(test, true);

	SlipPacket::ptr decode = SlipPacket::Create(encode->GetPacket(), false);


    EXPECT_EQ(encode->GetPacket(), decode->GetPacket());
}

TEST(SlipPacketTest, TestPacketStructure)
{
	std::vector<unsigned char> test = {'t','e','s','t','i','n','g'};
	SlipPacket::ptr encode = SlipPacket::Create(test, true);

	std::vector<unsigned char> verify = {SlipPacket::frame_boundary_octet, // start byte
			't','e','s','t','i','n','g', // payload
			226, 62, // payload crc
			SlipPacket::frame_boundary_octet}; // end byte
    EXPECT_EQ(verify, encode->GetPacket());
}


TEST(SlipPacketTest, TestEscapeChar)
{
	std::vector<unsigned char> test = {'t','e','s',SlipPacket::frame_boundary_octet,
			SlipPacket::frame_boundary_octet_replacement,SlipPacket::control_escape_octet,
			SlipPacket::control_escape_octet_replacement,'t','i','n','g'};
	SlipPacket::ptr encode = SlipPacket::Create(test, true);

	SlipPacket::ptr decode = SlipPacket::Create(encode->GetPacket(), false);


    EXPECT_EQ(encode->GetPacket(), decode->GetPacket());
    EXPECT_EQ(encode->GetPayload(), decode->GetPayload());

}

TEST(SlipPacketTest, TestParamPacketSlip)
{

	std::string payload = "/root/tmp<5>[8]/test[3]";
	Msg_Param paramPacket(msgAddr_t::FC_Param_e, 1,0,1,payload);


    BinaryOStream os;

    os << paramPacket;

	SlipPacket::ptr encode = SlipPacket::Create(os.get_internal_vec(), true);

	SlipPacket::ptr decode = SlipPacket::Create(encode->GetPacket(), false);


    EXPECT_EQ(encode->GetPacket(), decode->GetPacket());
    EXPECT_EQ(encode->GetPayload(), decode->GetPayload());
}
