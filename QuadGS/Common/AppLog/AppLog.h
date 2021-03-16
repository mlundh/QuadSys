/*
 * AppLog.h
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

#ifndef LOG_H_
#define LOG_H_

#include <sstream>
#include <vector>
#include "QGS_ModuleMsg.h"

#define LOG_ERROR(log, message) {log.setMsgLogLevel(log_level::error); log << message; log.flush();}
#define LOG_WARNING(log, message) {log.setMsgLogLevel(log_level::warning); log << message; log.flush();}
#define LOG_INFO(log, message) {log.setMsgLogLevel(log_level::info); log << message; log.flush();}
#define LOG_DEBUG(log, message) {log.setMsgLogLevel(log_level::debug); log << message; log.flush();}
#define LOG_MESSAGE_TRACE(log, message) {log.setMsgLogLevel(log_level::message_trace); log << message; log.flush();}


namespace QuadGS {


typedef std::function< void(std::unique_ptr<QGS_ModuleMsgBase> message) > sendFunction_t;

// Here we define our application severity levels.
enum log_level
{
    invalid,
	off,
    error,
    warning,
    info,
    debug,
    message_trace,
};

class AppLog;
//template <
//    typename T,
//    typename = decltype(std::declval<std::istringstream &>() >> std::declval<T &>(), void())>
template <
    typename T>
AppLog& operator<<(AppLog& applog, T const& t);

std::ostream &operator<<(std::ostream &strm, QuadGS::log_level const &level);


class AppLog
{
public:

    template <typename T>
    friend AppLog& operator<<(AppLog& applog, T const& t);

    AppLog(const std::string tag, sendFunction_t function);

    virtual ~AppLog();

    void setLogHandlerAddr(msgAddr_t addr);

    void flush();

    void setModuleLogLevel(QuadGS::log_level const svl);
    QuadGS::log_level getModuleLogLevel();
    
    void setMsgLogLevel(QuadGS::log_level const svl);
    std::string time_in_HH_MM_SS_MMM();

    static std::vector<std::string> mSeverityStrings;

protected: 
    std::string mTag;
    log_level mLogLevel = off;
    std::stringstream mMessage;
    log_level mMessageLogLevel = info;
    sendFunction_t mSendFunction;
    msgAddr_t mLoggerAddr;
    AppLog();
};


template <typename T>
AppLog& operator<<(AppLog& applog, T const& t)
{
    if(applog.mMessageLogLevel <= applog.mLogLevel)
    {
        applog.mMessage << t;
    }
    
    return applog;
}

} /* namespace QuadGS */

#endif /* LOG_H_ */
