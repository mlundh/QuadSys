/*
 * DataMsgTest.cpp
 *
 * Copyright (C) 2016 Martin Lundh
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

#include <vector>

#include "../../../QGS_Core/DataMsg/QCMsgHeader.h"
#include "../../../QGS_Core/DataMsg/QuadDebugMsg.h"
#include "../../../QGS_Core/DataMsg/QuadParamPacket.h"
#include "gtest/gtest.h"


using namespace QuadGS;
TEST(QCMsgHeaderTest, TestCreate)
{
	QCMsgHeader::ptr msg = QCMsgHeader::Create(1,1,1,0);
	BinaryOStream os;
	os << *msg;
	std::vector<uint8_t> v = {0x1, 0x81, 0x0, 0x0};
	EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(QCMsgHeaderTest, TestCreateFromArray)
{
	std::vector<uint8_t> v = {0x1, 0x81, 0x0, 0x0};
	// Create from array and normal constructor with equivalent data.
	QCMsgHeader::ptr msg = QCMsgHeader::Create(&v[0], v.size());
	QCMsgHeader::ptr msgVerify = QCMsgHeader::Create(1,1,1,0);
	BinaryOStream os;
	BinaryOStream osVerify;

	// Stream to an output stream. Verify that the resulting vector is equal.
	os << *msg;
	osVerify << *msgVerify;

	EXPECT_EQ(os.get_internal_vec(), osVerify.get_internal_vec());
	// Also verify that the resulting vector is equal to the original data.
	EXPECT_EQ(os.get_internal_vec(), v);

}


TEST(QCMsgHeaderTest, TestCreateFromArray2)
{
	std::vector<uint8_t> v = {0x1, 0x01, 0x55, 0x55};
	// Create from array and normal constructor with equivalent data.
	QCMsgHeader::ptr msg = QCMsgHeader::Create(&v[0], v.size());
	QCMsgHeader::ptr msgVerify = QCMsgHeader::Create(1,1,0,0x5555);
	BinaryOStream os;
	BinaryOStream osVerify;

	// Stream to an output stream. Verify that the resulting vector is equal.
	os << *msg;
	osVerify << *msgVerify;

	EXPECT_EQ(os.get_internal_vec(), osVerify.get_internal_vec());
	// Also verify that the resulting vector is equal to the original data.
	EXPECT_EQ(os.get_internal_vec(), v);

}

TEST(QCMsgHeaderTest, TestStreamThroughBase)
{
	std::vector<uint8_t> v = {0x1, 0x01, 0x55, 0x55};
	// Create from array and normal constructor with equivalent data.
	QCMsgHeader::ptr msg = QCMsgHeader::Create(&v[0], v.size());
	QCMsgHeader::ptr msgVerify = QCMsgHeader::Create(1,1,0,0x5555);
	BinaryOStream os;
	BinaryOStream osVerify;

	QuadGSMsg::QuadGSMsgPtr basePtr = std::static_pointer_cast<QuadGSMsg>(msg);
	// Stream to an output stream. Verify that the resulting vector is equal.
	os << *basePtr;
	osVerify << *msgVerify;

	EXPECT_EQ(os.get_internal_vec(), osVerify.get_internal_vec());
	// Also verify that the resulting vector is equal to the original data.
	EXPECT_EQ(os.get_internal_vec(), v);

}

TEST(QCMsgHeaderTest, ToString)
{
	QCMsgHeader::ptr msgVerify = QCMsgHeader::Create(0,1,1,0x5555);
	std::string test = msgVerify->toString();
	std::string verify = "[Parameters][IsResend][GetTree][Payload Size:21845]";
	EXPECT_EQ(test, verify);

}

TEST(QuadParamPacketTest, StreamOutIn)
{
	std::string payload = "/root/tmp<5>[8]/test[3]";
	QuadParamPacket::ptr paramPacket = QuadParamPacket::Create(payload,1,0);
	BinaryOStream os;
	os << *paramPacket;

	BinaryIStream is(os.get_internal_vec());

	QuadParamPacket::ptr paramPacketVerify = QuadParamPacket::Create();

	is >> *paramPacketVerify;


	EXPECT_EQ(paramPacketVerify->GetLastInSeq(), paramPacket->GetLastInSeq());
	EXPECT_EQ(paramPacketVerify->GetSequenceNumber(), paramPacket->GetSequenceNumber());
	EXPECT_EQ(paramPacketVerify->GetPayload(), paramPacket->GetPayload());
	EXPECT_EQ(paramPacketVerify->GetPayload(), payload);
}

TEST(QuadParamPacketTest, StreamOutIn2)
{
	std::string payload = "";
	QuadParamPacket::ptr paramPacket = QuadParamPacket::Create(payload,1,1);
	BinaryOStream os;
	os << *paramPacket;

	BinaryIStream is(os.get_internal_vec());

	QuadParamPacket::ptr paramPacketVerify = QuadParamPacket::Create();

	is >> *paramPacketVerify;


	EXPECT_EQ(paramPacketVerify->GetLastInSeq(), paramPacket->GetLastInSeq());
	EXPECT_EQ(paramPacketVerify->GetSequenceNumber(), paramPacket->GetSequenceNumber());
	EXPECT_EQ(paramPacketVerify->GetPayload(), paramPacket->GetPayload());
	EXPECT_EQ(paramPacketVerify->GetPayload(), payload);
}


TEST(QuadParamPacketTest, StreamOutIn3)
{
	// Raw data representing a param packet with sequenceNumber=1 and lastInSequence = 0, and
	// payload "/root/tmp<5>[8]/test[3]"
	uint8_t data[] = {1 , 47, 114, 111, 111, 116, 47, 116, 109, 112, 60,
			53, 62, 91, 56, 93, 47, 116, 101, 115, 116, 91, 51, 93};
	QuadParamPacket::ptr paramPacket = QuadParamPacket::Create(data, 24);

	std::string payload = "/root/tmp<5>[8]/test[3]";
	QuadParamPacket::ptr paramPacketVerify = QuadParamPacket::Create(payload,1,0);

	EXPECT_EQ(paramPacketVerify->GetLastInSeq(), paramPacket->GetLastInSeq());
	EXPECT_EQ(paramPacketVerify->GetSequenceNumber(), paramPacket->GetSequenceNumber());
	EXPECT_EQ(paramPacketVerify->GetPayload(), paramPacket->GetPayload());
	EXPECT_EQ(paramPacketVerify->GetPayload(), payload);
}


TEST(QuadDebugMsgTest, StreamOutIn)
{
	std::string payload = "debug:stuff to debug";
	QuadDebugMsg::ptr debugPacket = QuadDebugMsg::Create(payload);
	BinaryOStream os;
	os << *debugPacket;

	BinaryIStream is(os.get_internal_vec());

	QuadDebugMsg::ptr debugPacketVerify = QuadDebugMsg::Create();

	is >> *debugPacketVerify;

	EXPECT_EQ(debugPacketVerify->GetPayload(), debugPacket->GetPayload());
	EXPECT_EQ(debugPacketVerify->GetPayload(), payload);
}

TEST(QuadDebugMsgTest, StreamOutIn2)
{
	// Raw data representing a param packet  payload "debug:stuff to debug"
	uint8_t data[] = {100, 101, 98, 117, 103, 58, 115, 116, 117, 102, 102,
			 32, 116, 111, 32, 100, 101, 98, 117, 103};
	QuadDebugMsg::ptr paramPacket = QuadDebugMsg::Create(data, 20);

	std::string payload = "debug:stuff to debug";
	QuadDebugMsg::ptr paramPacketVerify = QuadDebugMsg::Create(payload);

	EXPECT_EQ(paramPacketVerify->GetPayload(), paramPacket->GetPayload());
	EXPECT_EQ(paramPacketVerify->GetPayload(), payload);
}
