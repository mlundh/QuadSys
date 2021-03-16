/*
 * Log.cpp
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

#include "AppLog.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include "Msg_GsAppLog.h"

namespace QuadGS
{

	std::vector<std::string> AppLog::mSeverityStrings =
		{
			"invalid",
			"off",
			"error",
			"warning",
			"info",
			"debug",
			"message_trace",
	};

	AppLog::AppLog(const std::string tag, sendFunction_t function)
		: mTag(tag), mSendFunction(function), mLoggerAddr(GS_Broadcast_e)
	{
	}

	AppLog::~AppLog()
	{
	}

    void AppLog::setLogHandlerAddr(msgAddr_t addr)
	{
		mLoggerAddr = addr;
	}	

	void AppLog::flush()
	{
		if (mMessageLogLevel <= mLogLevel)
		{
			std::stringstream ss;
			ss << "<" << time_in_HH_MM_SS_MMM() << "> " << "[" << mMessageLogLevel << "] " 
				<< "[" << mTag << "]: "
			    << mMessage.str() << std::endl;
			Msg_GsAppLog::ptr ptr = std::make_unique<Msg_GsAppLog>(mLoggerAddr, ss.str());
			mSendFunction(std::move(ptr)); // Send the message.
			mMessage.str(""); // Clear the stringstream
			mMessageLogLevel = info; // reset the logLevel to initial.
		}
	}
	// The operator puts a human-friendly representation of the severity level to the stream
	std::ostream &operator<<(std::ostream &strm, QuadGS::log_level const &level)
	{
		if (static_cast<std::size_t>(level) < AppLog::mSeverityStrings.size())
			strm << AppLog::mSeverityStrings[level];
		else
			strm << static_cast<int>(level);
		return strm;
	}

	void AppLog::setModuleLogLevel(QuadGS::log_level const svl)
	{
		mLogLevel = svl;
	}

	QuadGS::log_level AppLog::getModuleLogLevel()
	{
		return mLogLevel;
	}

	void AppLog::setMsgLogLevel(QuadGS::log_level const svl)
	{
		mMessageLogLevel = svl;
	}

	std::string AppLog::time_in_HH_MM_SS_MMM()
	{
		using namespace std::chrono;

		// get current time
		auto now = system_clock::now();

		// get number of milliseconds for the current second
		// (remainder after division into seconds)
		auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

		// convert to std::time_t in order to convert to std::tm (broken time)
		auto timer = system_clock::to_time_t(now);

		// convert to broken time
		std::tm bt = *std::localtime(&timer);

		std::ostringstream oss;

		oss << std::put_time(&bt, "%H:%M:%S"); // HH:MM:SS
		oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

		return oss.str();
	}
} /* namespace QuadGS */
