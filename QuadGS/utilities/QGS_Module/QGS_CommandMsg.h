/*
 * QGS_CommandMsg.h
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

#ifndef QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_COMMANDMSG_H_
#define QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_COMMANDMSG_H_
#include <string>
#include "QGS_ModuleMsg.h"
#include <vector>

namespace QuadGS {

class QGS_UiCommand_;

struct QGS_UiCommandDesc
{
public:
	QGS_UiCommandDesc(std::string name, std::string args, std::string doc);
	QGS_UiCommandDesc(const QGS_UiCommand_& uiCommand);
	virtual ~QGS_UiCommandDesc();
	std::string mName;           /* Human readable name of the function. */
	std::string mArgs;
	std::string mDoc;            /* Documentation for this function.  */
};



class QGS_CommandMsg: public QGS_ModuleMsg
{
public:
	typedef std::unique_ptr<QGS_CommandMsg> ptr;


	QGS_CommandMsg(std::string name, std::string args, std::string doc = "");

	QGS_CommandMsg(const QGS_ModuleMsg& moduleMsg);

	virtual ~QGS_CommandMsg();

	virtual BinaryIStream& stream(BinaryIStream& is);

	virtual BinaryOStream& stream(BinaryOStream& os) const;


	DISPATCH_FCN;

	QGS_UiCommandDesc mCommand;

};


class QGS_CommandRsltMsg: public QGS_ModuleMsg
{
public:
	typedef std::unique_ptr<QGS_CommandRsltMsg> ptr;


	QGS_CommandRsltMsg(std::string& result);

	QGS_CommandRsltMsg(const QGS_ModuleMsg& moduleMsg);

	virtual ~QGS_CommandRsltMsg();

	virtual BinaryIStream& stream(BinaryIStream& is);

	virtual BinaryOStream& stream(BinaryOStream& os) const;

	DISPATCH_FCN;

	std::string mResult;
};


class QGS_CommandReqMsg: public QGS_ModuleMsg
{
public:
	typedef std::unique_ptr<QGS_CommandReqMsg> ptr;


	QGS_CommandReqMsg();

	QGS_CommandReqMsg(const QGS_ModuleMsg& moduleMsg);

	virtual ~QGS_CommandReqMsg();

	virtual BinaryIStream& stream(BinaryIStream& is);

	virtual BinaryOStream& stream(BinaryOStream& os) const;

	DISPATCH_FCN;
};


class QGS_CommandReqRspMsg: public QGS_ModuleMsg
{
public:
	typedef std::unique_ptr<QGS_CommandReqRspMsg> ptr;


	QGS_CommandReqRspMsg(std::vector<QGS_UiCommand_>& Commands);

	QGS_CommandReqRspMsg(const QGS_ModuleMsg& moduleMsg);

	virtual ~QGS_CommandReqRspMsg();

	virtual BinaryIStream& stream(BinaryIStream& is);

	virtual BinaryOStream& stream(BinaryOStream& os) const;

	DISPATCH_FCN;

	std::vector<QGS_UiCommandDesc> mCommands;
};


} /* namespace QuadGS */

#endif /* QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_COMMANDMSG_H_ */
