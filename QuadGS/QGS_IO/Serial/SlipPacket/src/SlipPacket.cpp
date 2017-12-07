/*
 * SlipPacket.cpp
 *
 * Copyright (C) 2015 Martin Lundh
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

#include "../../../../QGS_IO/Serial/SlipPacket/SlipPacket.h"

#include <iostream>
#include <sstream>
#include <string>

extern "C"
{
#include "../../../../QGS_IO/Serial/crc/crc.h"
}

namespace QuadGS {

bool SlipPacket::mCrcInit(false);





SlipPacket::SlipPacket(const data_t& data, bool isPayload)
{
	initCrc();
	if(isPayload)
	{
		mPayload = data;
		mPacket.reserve(255);
		Encode();
	}
	else
	{
		if((data[0] == frame_boundary_octet) && (data[data.size() - 1] == frame_boundary_octet))
		{
			mPacket = data;
			mPayload.reserve(255);
			Decode();
		}
		else
		{
			throw std::runtime_error("Not a valid slip packet.");
		}
	}

}

SlipPacket::SlipPacket(const uint8_t* data, uint8_t dataLength, bool isPayload)
{
	initCrc();
	if(isPayload)
	{
		mPayload.assign(data, data + dataLength);
		mPacket.reserve(255);
		Encode();
	}
	else
	{
		if((data[0] == frame_boundary_octet) && (data[dataLength - 1] == frame_boundary_octet))
		{
			mPacket.assign(data, data + dataLength);
			mPayload.reserve(255);
			Decode();
		}
		else
		{
			throw std::runtime_error("Not a valid slip packet.");
		}
	}
}

SlipPacket::~SlipPacket()
{
}


void SlipPacket::initCrc()
{
	if(!mCrcInit)
	{
		crcInit();
		mCrcInit = true;
	}
}
SlipPacket::ptr SlipPacket::Create(const uint8_t* data, uint8_t dataLength, bool isPayload)
{
	if((!data) || !(dataLength > 0) || !(dataLength < 255))
	{
		throw std::runtime_error("Payload length out of reach.");
	}
	//ensure it is a valid SLIP packet and then create a new object.
	ptr p( new SlipPacket( data, dataLength, isPayload )  );
	return p;
}
SlipPacket::ptr SlipPacket::Create(const data_t& data, bool isPayload)
{
	if(data.size())
	{
		ptr p( new SlipPacket( data, isPayload ) );
		return p;
	}
	else
	{
		throw std::runtime_error("Trying to create empty slip packet.");
	}
}

SlipPacket::data_t& SlipPacket::GetPayload()
{
	return mPayload;
}
SlipPacket::data_t& SlipPacket::GetPacket()
{
	return mPacket;
}

// TODO check and add control!
bool SlipPacket::Encode()
{

	addChecksumToPayload();

	if(!mPacket.empty())
	{
		throw std::runtime_error("Packet not empty when encoding!");
	}
	mPacket.push_back(SlipControlOctets::frame_boundary_octet);

	uint16_t i;
	for ( i = 0; i < (mPayload.size()); i++)
	{
		if ( mPayload[i] == frame_boundary_octet )
		{
			mPacket.push_back(control_escape_octet);
			mPacket.push_back(frame_boundary_octet_replacement);
		}
		else if ( mPayload[i] == control_escape_octet )
		{
			mPacket.push_back(control_escape_octet);
			mPacket.push_back(control_escape_octet_replacement);
		}
		else
		{
			mPacket.push_back(mPayload[i]);
		}
	}
	mPacket.push_back(frame_boundary_octet);

	if(!verifyChecksum())
	{
		throw std::runtime_error("Crc error!");
	}

	return true;
}

bool SlipPacket::Decode()
{
	uint16_t k;
	if(mPacket[0] != SlipControlOctets::frame_boundary_octet)
	{
		throw std::runtime_error("Not a valid Slip Packet");
	}
	if(!mPayload.empty())
	{
		throw std::runtime_error("Payload not empty when decoding!");
	}
	for (k = 1; k < (mPacket.size() - 1); k++ )
	{
		if ( mPacket[k] == control_escape_octet )
		{
			k++;
			if ( mPacket[k] == control_escape_octet_replacement )
			{
				mPayload.push_back(control_escape_octet);
			}
			else if ( mPacket[k] == frame_boundary_octet_replacement )
			{
				mPayload.push_back(frame_boundary_octet);
			}
			else
			{
				throw std::runtime_error("Not a valid Slip Packet");
			}
		}
		else
		{
			mPayload.push_back(mPacket[k]);
		}
	}
	if(!verifyChecksum())
	{
		throw std::runtime_error("Crc error!");
	}
	return true;
}

void SlipPacket::addChecksumToPayload()
{
	// Add CRC16 checksum to the payload.
	uint16_t plLength = mPayload.size();
	uint16_t msg_crc = static_cast<uint16_t>(crcFast(&mPayload[0], plLength));
	mPayload.push_back(static_cast<uint8_t>((msg_crc >> 8) & 0Xff));
	mPayload.push_back(static_cast<uint8_t>((msg_crc) & 0Xff));
}
bool SlipPacket::verifyChecksum()
{
	uint16_t plLength = mPayload.size();
	uint16_t calc_crc = static_cast<uint16_t>(crcFast(&mPayload[0], plLength - 2 ));

	unsigned int msg_crc = 0;
	msg_crc |=  mPayload[plLength - 2] << 8;
	msg_crc |=  mPayload[plLength - 1];
	mPayload.erase(mPayload.end()-2, mPayload.end());// remove CRC, it is not a part of the QSP.
	return (static_cast<uint16_t>(msg_crc) == calc_crc);
}

} /* namespace QuadGS */
