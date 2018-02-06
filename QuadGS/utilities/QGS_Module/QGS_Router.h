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

class QGS_Router : public QGS_MessageHandler<QGS_ModuleSubMsg>
{
public:
	QGS_Router(std::string name);
	virtual ~QGS_Router();

	void bind(QGS_Module* module);

	bool done();
protected:

	void incomingPort(QGS_ModuleMsg::ptr message, int port);


private:

	void sendMsg(QGS_ModuleMsg::ptr message);

	void internalSend(QGS_ModuleMsg::ptr message, int port);

	void runRouter();

	void checkUniqueName(std::string &name);

	void route(QGS_ModuleMsg::ptr fifoEntry);

	virtual void process(QGS_ModuleSubMsg* msg);

	ThreadSafeFifo<QGS_ModuleMsg::ptr> mFifo; // ,essage, originating port
	std::vector<std::string> mPortNameMaping; // map port number to name of module connected there.
	std::map<messageTypes_t, std::list<int> > mSubscriptions; // Map message types to ports subscribed to that message type.
	std::vector<WriteFcn> mWriteFunctions;
    std::thread mThread;
	unsigned int mNrModules;
    std::string mName;
    bool mStop;



};

} /* namespace QuadGS */

#endif /* QUADGS_UTILITIES_QGS_MODULE_SRC_QGS_ROUTER_H_ */
