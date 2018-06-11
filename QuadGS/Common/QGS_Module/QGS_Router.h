/*
 * QGSRouter.h
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

#ifndef QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_ROUTER_H_
#define QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_ROUTER_H_


#include <map>
#include <list>
#include <thread>
#include "ThreadSafeFifo.hpp"
#include "QGS_ModuleMsg.h"
#include "AppLog.h"
namespace QuadGS {

class QGS_Module;

class QGS_Router
{
public:
	QGS_Router(std::string name);
	virtual ~QGS_Router();

	void bind(QGS_Module* module);

	bool done();
protected:

	void incomingPort(QGS_ModuleMsgBase::ptr message);


private:

	void sendMsg(QGS_ModuleMsgBase::ptr message);

	void internalSend(QGS_ModuleMsgBase::ptr message, std::string port, bool broadcast);

	void runRouter();

	void checkUniqueName(std::string &name);

	void route(QGS_ModuleMsgBase::ptr fifoEntry);

	ThreadSafeFifo<QGS_ModuleMsgBase::ptr> mFifo;
	std::map<std::string, WriteFcn> mWriteFunctions;
    std::thread mThread;
	unsigned int mNrModules;
    std::string mName;
    bool mStop;
	AppLog mLogger;



};

} /* namespace QuadGS */

#endif /* QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_ROUTER_H_ */
