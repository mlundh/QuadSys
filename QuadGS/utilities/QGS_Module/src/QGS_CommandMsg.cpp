/*
 * QGS_CommandMsg.cpp
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

#include "QGS_CommandMsg.h"
#include "QGS_Module.h"
namespace QuadGS {

BinaryOStream& operator << (BinaryOStream& ostm, const QGS_UiCommandDesc& val)
{
	ostm << val.mName;
	ostm << val.mArgs;
	ostm << val.mDoc;

	return ostm;
}



BinaryIStream& operator >> (BinaryIStream& istm, QGS_UiCommandDesc& val)
{
	istm >> val.mName;
	istm >> val.mArgs;
	istm >> val.mDoc;
	return istm;
}





QGS_UiCommandDesc::QGS_UiCommandDesc(std::string name, std::string args, std::string doc)
:mName(name)
,mArgs(args)
,mDoc(doc)
{}

QGS_UiCommandDesc::QGS_UiCommandDesc(const QGS_UiCommandDesc& uiCommandDesc)
:mName(uiCommandDesc.mName)
,mArgs(uiCommandDesc.mArgs)
,mDoc(uiCommandDesc.mDoc)
{

}

QGS_UiCommandDesc::QGS_UiCommandDesc(const QGS_UiCommand_& uiCommand)
:mName(uiCommand.mName)
,mArgs()
,mDoc(uiCommand.mDoc)
{}
QGS_UiCommandDesc::~QGS_UiCommandDesc()
{
}



QGS_CommandMsg::QGS_CommandMsg(std::string name, std::string args)
:QGS_ModuleMsg(msgCommand)
,mCommand(name, args, "")
{

}

QGS_CommandMsg::QGS_CommandMsg(const QGS_CommandMsg& msg)
:QGS_ModuleMsg(msg)
,mCommand(msg.mCommand)

{

}

QGS_CommandMsg::QGS_CommandMsg(const QGS_ModuleMsgBase& header)
:QGS_ModuleMsg(header)
,mCommand("", "", "")

{

}


QGS_CommandMsg::~QGS_CommandMsg()
{
}



BinaryIStream& QGS_CommandMsg::stream(BinaryIStream& is)
{
	is >> mCommand;
	return is;
}

BinaryOStream& QGS_CommandMsg::stream(BinaryOStream& os) const
{
	QGS_ModuleMsg::stream(os);
	os << mCommand;
	return os;
}


/*-----------------------------*/


QGS_CommandRsltMsg::QGS_CommandRsltMsg(std::string& result)
:QGS_ModuleMsg(msgCommandRslt)
,mResult(result)
{

}

QGS_CommandRsltMsg::QGS_CommandRsltMsg(const QGS_CommandRsltMsg& msg)
:QGS_ModuleMsg(msg)
,mResult(msg.mResult)
{

}


QGS_CommandRsltMsg::QGS_CommandRsltMsg(const QGS_ModuleMsgBase& header)
:QGS_ModuleMsg(header)
,mResult()
{

}


QGS_CommandRsltMsg::~QGS_CommandRsltMsg()
{
}



BinaryIStream& QGS_CommandRsltMsg::stream(BinaryIStream& is)
{
	is >> mResult;

	return is;
}

BinaryOStream& QGS_CommandRsltMsg::stream(BinaryOStream& os) const
{
	QGS_ModuleMsg::stream(os);
	os << mResult;

	return os;
}


/*----------------------------*/


QGS_CommandReqMsg::QGS_CommandReqMsg()
:QGS_ModuleMsg(msgCommandReq)
{

}

QGS_CommandReqMsg::QGS_CommandReqMsg(const QGS_CommandReqMsg& msg)
:QGS_ModuleMsg(msg)
{

}

QGS_CommandReqMsg::QGS_CommandReqMsg(const QGS_ModuleMsgBase& header)
:QGS_ModuleMsg(header)
{

}


QGS_CommandReqMsg::~QGS_CommandReqMsg()
{
}


BinaryIStream& QGS_CommandReqMsg::stream(BinaryIStream& is)
{
	return is;
}

BinaryOStream& QGS_CommandReqMsg::stream(BinaryOStream& os) const
{
	QGS_ModuleMsg::stream(os);
	return os;
}



/*--------------------------------------*/


QGS_CommandReqRspMsg::QGS_CommandReqRspMsg(std::vector<QGS_UiCommand_>& Commands)
:QGS_ModuleMsg(msgCommandRslt)
{
	for(auto i : Commands)
	{
		mCommands.push_back(QGS_UiCommandDesc(i));
	}
}

QGS_CommandReqRspMsg::QGS_CommandReqRspMsg(const QGS_ModuleMsgBase& header)
:QGS_ModuleMsg(header)
{

}

QGS_CommandReqRspMsg::QGS_CommandReqRspMsg(const QGS_CommandReqRspMsg& msg)
:QGS_ModuleMsg(msg)
{
	for(auto i : msg.mCommands)
	{
		mCommands.push_back(i);
	}
}


QGS_CommandReqRspMsg::~QGS_CommandReqRspMsg()
{
}



BinaryIStream& QGS_CommandReqRspMsg::stream(BinaryIStream& is)
{
	size_t size = 0;
	is >> size;
	for(unsigned int i = 0; i < size; i++)
	{
		QGS_UiCommandDesc desc("","","");
		is >> desc;
		mCommands.push_back(desc);
	}
	return is;
}

BinaryOStream& QGS_CommandReqRspMsg::stream(BinaryOStream& os) const
{
	QGS_ModuleMsg::stream(os);
	os << mCommands.size();
	for(unsigned int i = 0; i < mCommands.size(); i++)
	{
		os << mCommands[i];
	}
	return os;
}



} /* namespace QuadGS */
