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

#include <memory>

#include "QGS_Msg.h"
#include "QGS_DebugMsg.h"
#include "QGS_LogMsg.h"
#include "QGS_MsgHeader.h"
#include "QGS_ParamMsg.h"

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
    mHeader = QGS_MsgHeader::Create(0,0,0,0);
    is >> *mHeader;
    switch (mHeader->GetAddress())
    {
    case QGS_MsgHeader::addresses::Parameters:
    {
        QGSParamMsg::ptr tmp = QGSParamMsg::Create();
        is >> *tmp;
        mPayload = tmp;
        break;
    }
    case QGS_MsgHeader::addresses::State:
    {
        return -1;
        break;
    }
    case QGS_MsgHeader::addresses::Log:
    {
        QGS_LogMsg::ptr tmp = QGS_LogMsg::Create();
        is >> *tmp;
        mPayload = tmp;
        break;
    }
    case QGS_MsgHeader::addresses::Status:
    {
        mPayload.reset();
        break;
    }
    case QGS_MsgHeader::addresses::Debug:
    {
        QGS_DebugMsg::ptr tmp = QGS_DebugMsg::Create();
        is >> *tmp;
        mPayload = tmp;
        break;
    }
    default:
        break;
    }

    return 0;
}

QGS_MsgHeader::ptr FcParser::getHeader( void )
{
    QGS_MsgHeader::ptr ptr = mHeader;
    mHeader.reset();
    return ptr;
}

QGS_Msg::Ptr FcParser::getPayload( void )
{
    QGS_Msg::Ptr ptr = mPayload;
    mPayload.reset();
    return ptr;
}

} /* namespace QuadGS */
