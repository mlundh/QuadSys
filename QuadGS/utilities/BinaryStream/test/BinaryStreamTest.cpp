/*
 * main.cpp
 *
 *  Created on: Feb 5, 2017
 *      Author: martin
 */
#include "BinaryStream.h"

#include "gtest/gtest.h"

using namespace QuadGS;
TEST(BinaryOStreamTest, StreamInteger)
{
	BinaryOStream os;
	os << (int)5;
	std::vector<unsigned char> v = {0, 0, 0, 5};
    EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, Streamuint16)
{
	BinaryOStream os;
	os << (uint16_t)5;
	std::vector<unsigned char> v = {0, 5};
    EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, Streamuint64)
{
	BinaryOStream os;
	os << (uint64_t)0xFEEDFEEDFEEDFEED;
	std::vector<unsigned char> v = {0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED};
    EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, BitWidthSetting)
{
	BinaryOStream os;
	os << SetBits(5);
	EXPECT_EQ(os.mNextWidth, 5);
}

TEST(BinaryOStreamTest, StreamOneBit)
{
	BinaryOStream os;
	os << SetBits(1);
	os << 0xff;
	std::vector<unsigned char> v = {0x80};
	EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, StreamTwoBits)
{
	BinaryOStream os;
	os << SetBits(2);
	os << 0xff;
	std::vector<unsigned char> v = {0xc0};
	EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, StreamEightBits)
{
	BinaryOStream os;
	os << SetBits(8);
	os << 0xffff;
	std::vector<unsigned char> v = {0xff};
	EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, StreamTwice)
{
	BinaryOStream os;
	os << SetBits(1);
	os << 0xffff;
	os << SetBits(1);
	os << 0xffff;
	std::vector<unsigned char> v = {0xc0};
	EXPECT_EQ(os.get_internal_vec(), v);
}


TEST(BinaryOStreamTest, StreamTwiceDifferentBytes)
{
	// Write two consecutive bytes, in two passes.
	BinaryOStream os;
	os << SetBits(8);
	os << 0xffff;
	os << SetBits(8);
	os << 0xff55;
	std::vector<unsigned char> v = {0xFF, 0x55};
	EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, StreamMultipleTimes)
{
	// each write keeps within byte boundary's.
	BinaryOStream os;
	os << SetBits(4) << 0xffff;
	os << SetBits(4) << 0xff55;
	os << SetBits(5) << 0xeeee;
	os << SetBits(3) << 0x2222;
	std::vector<unsigned char> v = {0xF5, 0x72};
	EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, StreamOffsetUint8)
{
	// Stream a total of 9 bits.
	BinaryOStream os;
	os << SetBits(1) << 0x0;
	os << (uint8_t)0xff;
	std::vector<unsigned char> v = {0x7F, 0x80};
	EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, StreamOffsetUint16)
{
	// write two bytes, into buffer with one bit in it.
	BinaryOStream os;
	os << SetBits(1) << 0x0;
	os << (uint16_t)0x10ff;
	std::vector<unsigned char> v = {0x8, 0x7F, 0x80};
	EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, StreamTwoOffsetUint32)
{
	// write two bytes, into buffer with onw bit in it.
	BinaryOStream os;
	os << SetBits(1) << 0x0;
	int test = 0xff6666f2;
	os << test;
	os << SetBits(1) << 0x0;
	os << test;
	std::vector<unsigned char> v = {0x7F, 0xB3, 0x33, 0x79, 0x3F, 0xD9, 0x99, 0xBC, 0x80};
	EXPECT_EQ(os.get_internal_vec(), v);
}

TEST(BinaryOStreamTest, StreamStrangeSizes)
{
	// write two bytes, into buffer with onw bit in it.
	BinaryOStream os;
	os << SetBits(1) << 0x0;
	int test = 0xff6666f3;
	os << SetBits(31) << test;
	std::vector<unsigned char> v = {0x7F, 0x66, 0x66, 0xF3};
	EXPECT_EQ(os.get_internal_vec(), v);
}


TEST(BinaryOStreamTest, StreamChar)
{
	BinaryOStream os;

	std::string test = "TestVector";
	os << test.c_str();
	// Comparing strings are easier than comparing c_strings, so lets convert.
	// (This adds a null terminator, that is ok as long as we know...)
	std::string verify((char*)&os.get_internal_vec()[0], os.get_internal_vec().size());
	EXPECT_EQ(test, verify);
}

TEST(BinaryOStreamTest, StreamString)
{
	BinaryOStream os;

	std::string test = "TestVector";
	os << test;
	// Comparing strings are easier than comparing c_strings, so lets convert.
	std::string verify((char*)&os.get_internal_vec()[0], os.get_internal_vec().size());
	EXPECT_EQ(test, verify);
}

// TODO signed integer test!

TEST(BinaryIStreamTest, StreamInteger)
{

	std::vector<unsigned char> v = {0x05, 0x00, 0x00, 0x5};
	BinaryIStream is(v);
	int i = 0;
	is >> i;
    EXPECT_EQ(i, 0x05000005);
}

TEST(BinaryIStreamTest, StreamUint8)
{

	std::vector<unsigned char> v = {5};
	BinaryIStream is(v);
	uint8_t i = 0;
	is >> i;
    EXPECT_EQ(i, 5);
}

TEST(BinaryIStreamTest, StreamUint16)
{

	std::vector<unsigned char> v = {0x05,0x05};
	BinaryIStream is(v);
	uint16_t i = 0;
	is >> i;
    EXPECT_EQ(i, 0x505);
}
TEST(BinaryIStreamTest, StreamUint32)
{

	std::vector<unsigned char> v = {0xFE, 0xED, 0xFE, 0xED};
	BinaryIStream is(v);
	uint32_t i = 0;
	is >> i;
    EXPECT_EQ(i, 0xFEEDFEED);
}

TEST(BinaryIStreamTest, StreamUint64)
{

	std::vector<unsigned char> v = {0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED};
	BinaryIStream is(v);
	uint64_t i = 0;
	is >> i;
    EXPECT_EQ(i, 0xFEEDFEEDFEEDFEED);
}


TEST(BinaryIStreamTest, StreamOneBit)
{

	std::vector<unsigned char> v = {0xFF,0x05};
	BinaryIStream is(v);
	uint16_t i = 0;
	is >> SetBits(1);
	is >> i;
    EXPECT_EQ(i, 0x1);
}

TEST(BinaryIStreamTest, StreamOneBitMulti)
{

	std::vector<unsigned char> v = {0xA0,0x05};
	BinaryIStream is(v);
	int i = -1;
	int j = -1;
	int k = -1;
	is >> SetBits(1) >> i;
	is >> SetBits(1) >> j;
	is >> SetBits(1) >> k;

    EXPECT_EQ(i, 0x1);
    EXPECT_EQ(j, 0x0);
    EXPECT_EQ(k, 0x1);
}


TEST(BinaryIStreamTest, Streamuint8Twice)
{

	std::vector<unsigned char> v = {0xFF,0x05};
	BinaryIStream is(v);
	uint8_t i = 0;
	uint8_t j = 0;
	is >> i >> j;
    EXPECT_EQ(i, 0xFF);
    EXPECT_EQ(j, 0x05);
}

TEST(BinaryIStreamTest, Streamuint8TwiceOffset)
{
	std::vector<unsigned char> v = {0xFF,0x05};
	BinaryIStream is(v);
	uint8_t i = 0;
	uint8_t j = 0;
	is >> SetBits(1) >> i;
	is >> SetBits(7) >> i;
	is >> SetBits(8) >> j;
	EXPECT_EQ(i, 0x7F);
    EXPECT_EQ(j, 0x05);
}

TEST(BinaryIStreamTest, Streamuint15Offset)
{
	std::vector<unsigned char> v = {0x33, 0xFF,0x05};
	BinaryIStream is(v);
	uint8_t i = 0;
	uint32_t j = 0;
	uint8_t k = 0;
	is >> SetBits(4) >> i;
	is >> SetBits(15) >> j;
	is >> SetBits(5) >> k;
	EXPECT_EQ(j, (unsigned)0x1FF8);
	EXPECT_EQ(i, (unsigned)0x3);
	EXPECT_EQ(k, (unsigned)0x5);
}

TEST(BinaryIStreamTest, Streamuint12Offset)
{
	std::vector<unsigned char> v = {0x33, 0xFF,0x05};
	BinaryIStream is(v);
	uint8_t i = 0;
	uint32_t j = 0;
	is >> SetBits(2) >> i;
	is >> SetBits(12) >> j;
	EXPECT_EQ(i, (unsigned)0x0);
	EXPECT_EQ(j, (unsigned)0xCFF);
}


TEST(BinaryIStreamTest, StreamString)
{
	std::vector<unsigned char> v = {0x54, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67};

	BinaryIStream is(v);

	std::string test;
	is >> test;

	std::string verify("Testing");

	EXPECT_EQ(verify, test);
}

TEST(BinaryIOStreamTest, StremOutThenIn)
{
	BinaryOStream os;
	int a = 0x54;
	int b = 0xAFD;
	int c = 1;
	int d = 0xDEED;
	os << SetBits(16) << a;
	os << SetBits(32) << b;
	os << SetBits(1) << c;
	os << SetBits(32) << d;

	BinaryIStream is(os.get_internal_vec());

	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;

	is >> SetBits(16) >> i;
	is >> SetBits(32) >> j;
	is >> SetBits(1) >> k;
	is >> SetBits(32) >> l;

	EXPECT_EQ(i, a);
	EXPECT_EQ(j, b);
	EXPECT_EQ(k, c);
	EXPECT_EQ(d, l);

}

TEST(BinaryIOStreamTest, StremOutThenIn2)
{
	BinaryOStream os;
	int a = 586;
	int b = -0xAFD;
	int c = 1;
	int d = -0xDEED;
	os << SetBits(16) << a;
	os << SetBits(32) << b;
	os << SetBits(1) << c;
	os << SetBits(32) << d;

	BinaryIStream is(os.get_internal_vec());

	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;

	is >> SetBits(16) >> i;
	is >> SetBits(32) >> j;
	is >> SetBits(1) >> k;
	is >> SetBits(32) >> l;

	EXPECT_EQ(i, a);
	EXPECT_EQ(j, b);
	EXPECT_EQ(k, c);
	EXPECT_EQ(d, l);

}

TEST(BinaryIOStreamTest, StremOutThenIn64bit)
{
	BinaryOStream os;

	uint64_t d = 0xFEEDFEEDFEEDFEED;
	os << SetBits(64) << d;

	BinaryIStream is(os.get_internal_vec());

	uint64_t l = 0;
	is >> SetBits(64) >> l;

	EXPECT_EQ(l, d);
}

TEST(BinaryIOStreamTest, StremOutThenIn1Plus64bit)
{
	BinaryOStream os;
	int c = 1;
	uint64_t d = 0xFEEDFEEDFEEDFEED;

	os << SetBits(1) << c;
	os << SetBits(64) << d;

	BinaryIStream is(os.get_internal_vec());


	int k = 0;
	uint64_t l = 0;

	is >> SetBits(1) >> k;
	is >> SetBits(64) >> l;

	EXPECT_EQ(k, c);
	EXPECT_EQ(l, d);

}

TEST(BinaryIOStreamTest, StremOutThenInMultipleSizes)
{
	BinaryOStream os;
	int a = 586;
	int b = -0xAFD;
	int c = 1;
	uint64_t d = 0xFEEDFEEDFEEDFEED;
	os << SetBits(16) << a;
	os << SetBits(32) << b;
	os << SetBits(1) << c;
	os << SetBits(64) << d;

	BinaryIStream is(os.get_internal_vec());

	int i = 0;
	int j = 0;
	int k = 0;
	uint64_t l = 0;

	is >> SetBits(16) >> i;
	is >> SetBits(32) >> j;
	is >> SetBits(1) >> k;
	is >> SetBits(64) >> l;

	EXPECT_EQ(i, a);
	EXPECT_EQ(j, b);
	EXPECT_EQ(k, c);
	EXPECT_EQ(l, d);

}


TEST(BinaryIOStreamTest, StremOutThenInMultipleSizes2)
{
	BinaryOStream os;
	int a = 586;
	int b = -0xAFD;
	int c = 1;
	uint64_t d = 0xFEEDFEEDFEEDFEED;
	os << SetBits(1) << c;
	os << SetBits(16) << a;
	os << SetBits(32) << b;
	os << SetBits(64) << d;

	BinaryIStream is(os.get_internal_vec());

	int i = 0;
	int j = 0;
	int k = 0;
	uint64_t l = 0;

	is >> SetBits(1) >> k;

	is >> SetBits(16) >> i;
	is >> SetBits(32) >> j;
	is >> SetBits(64) >> l;

	EXPECT_EQ(i, a);
	EXPECT_EQ(j, b);
	EXPECT_EQ(k, c);
	EXPECT_EQ(l, d);

}

// TODO use parameterized tests!

