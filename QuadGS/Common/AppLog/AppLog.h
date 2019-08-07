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

#include <cassert>
#include <iostream>
#include <fstream>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/support/date_time.hpp>


namespace QuadGS {

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;
using boost::shared_ptr;

// Here we define our application severity levels.
enum severity_level
{
	invalid,
	off,
    error,
    warning,
    info,
    debug,
    message_trace,
};


std::ostream& operator<<( std::ostream& strm, QuadGS::severity_level level);

class AppLog
{
public:
    AppLog();
    AppLog(const std::string tag);
    virtual ~AppLog();
    static void Init( std::string FilenameAppLog, std::string FilenameMsgLog, std::ostream& outstream, severity_level svl, bool msg);
    void QuadLog(QuadGS::severity_level svl, std::string msg);
    static QuadGS::severity_level logLevelFromStr(std::string level);
    static std::string setLogLevelFromStr(std::string level);

    static void logLevel(QuadGS::severity_level const svl);

    static std::vector<std::string> mSeverityStrings;

protected:
    src::severity_logger<QuadGS::severity_level> slg;
};

} /* namespace QuadGS */

#endif /* LOG_H_ */
