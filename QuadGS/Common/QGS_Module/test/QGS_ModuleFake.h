/*
 * QGS_ModuleFake.h
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

#ifndef QUADGS_UTILITIES_QGS_MODULE_TEST_QGS_MODULEFAKE_H_
#define QUADGS_UTILITIES_QGS_MODULE_TEST_QGS_MODULEFAKE_H_
#include <atomic>
#include "QGS_Module.h"
#include "Msg_Param.h"
#include "Msg_DebugIo.h"

namespace QuadGS {

class FakeModule
		: public QGS_ReactiveModule
		, public QGS_MessageHandler<Msg_Param>
		, public QGS_MessageHandler<Msg_DebugIo>

{
public:
	FakeModule(std::string name);

	virtual ~FakeModule();

	int getNrMsg();

	void sendDummyDebugMsg(std::string dest);

	void sendDummyParamMsg(std::string dest);

	void returnNxtMsg(bool flag);

	std::string testCommand(std::string);
private:

	virtual void process_internal(QGS_ModuleMsgBase* ptr);
	virtual void process(Msg_Param* message);
	virtual void process(Msg_DebugIo* message);

	std::atomic<int> mNrMsg;
	std::atomic<int> mReturnNxtMsg;
};




class QGS_ThreadedModuleFake
		: public QGS_ThreadedModule
		, public QGS_MessageHandler<Msg_DebugIo>
		, public QGS_MessageHandler<Msg_Param>
{
public:
	QGS_ThreadedModuleFake(std::string name);

	virtual ~QGS_ThreadedModuleFake();

	int getNrMsg();

	void subscribeMsg(messageTypes_t type);

	void sendDummyDebugMsg(std::string dest);

	void sendDummyParamMsg(std::string dest);

	void returnNxtMsg(bool flag);
private:

	virtual void process_internal(QGS_ModuleMsgBase* ptr);
	virtual void process(Msg_DebugIo* message);
	virtual void process(Msg_Param* message);

	virtual void module();

	std::atomic<int> mNrMsg;
	std::atomic<int> mReturnNxtMsg;
};

} /* namespace QuadGS */

#endif /* QUADGS_UTILITIES_QGS_MODULE_TEST_QGS_MODULEFAKE_H_ */
