/*
 * FcParser.cpp
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

#include "FcParser.h"
#include "QuadGSMsg.h"
#include "QCMsgHeader.h"
#include "QuadParamPacket.h"
#include "QuadDebugMsg.h"
#include <memory>

namespace QuadGS
{


FcParser::FcParser():ParserBase()
{

}

FcParser::~FcParser()
{
}


int FcParser::parse( std::shared_ptr<std::vector<unsigned char> > data)
{
	//Package into a slip packet and send the packeged data.
	BinaryIStream is(*data);
	mHeader = QCMsgHeader::Create(0,0,0,0);
	is >> *mHeader;
	// TODO switch on header!!
	switch (mHeader->GetAddress())
	{
	case QCMsgHeader::addresses::Parameters:
	{
		QuadParamPacket::ptr tmp = QuadParamPacket::Create();
		is >> *tmp;
		mPayload = tmp;
		break;
	}
	case QCMsgHeader::addresses::FunctionCall:
	{
		return -1;
		break;
	}
	case QCMsgHeader::addresses::Status:
	{
		mPayload.reset();
		break;
	}
	case QCMsgHeader::addresses::Debug:
	{
		QuadDebugMsg::ptr tmp = QuadDebugMsg::Create();
		is >> *tmp;
		mPayload = tmp;
		break;
	}
	default:
    	break;
    }

    return 0;
}

QCMsgHeader::ptr FcParser::getHeader( void )
{
	QCMsgHeader::ptr ptr = mHeader;
	mHeader.reset();
	return ptr;
}

QuadGSMsg::QuadGSMsgPtr FcParser::getPayload( void )
{
	QuadGSMsg::QuadGSMsgPtr ptr = mPayload;
	mPayload.reset();
	return ptr;
}

} /* namespace QuadGS */
