/*
 * Log.h
 *
 *  Created on: Jan 31, 2015
 *      Author: martin
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
    normal,
    notification,
    warning,
    error,
    critical
};

std::ostream& operator<<( std::ostream& strm, QuadGS::severity_level level);

class Log
{
public:
    Log();
    virtual ~Log();
    static void Init( std::string FilenameAppLog, std::string FilenameMsgLog );

protected:
    src::severity_logger< QuadGS::severity_level > slg;
};

} /* namespace QuadGS */

#endif /* LOG_H_ */
