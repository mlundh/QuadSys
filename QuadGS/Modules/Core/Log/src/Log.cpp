/*
 * Log.cpp
 *
 *  Created on: Jan 31, 2015
 *      Author: martin
 */

#include "Log.h"
#include <boost/core/null_deleter.hpp>
namespace QuadGS {


Log::Log()
{

}

Log::Log(std::string tag)
{
  slg.add_attribute("Tag", attrs::constant< std::string >(tag));
}

Log::~Log()
{

}

void Log::QuadLog(QuadGS::severity_level svl, std::string msg)
{
    logging::record rec = slg.open_record(keywords::severity = svl);
    if (rec)
    {
        logging::record_ostream strm(rec);
        strm << msg;
        strm.flush();
        slg.push_record(boost::move(rec));
    }
}

// The operator puts a human-friendly representation of the severity level to the stream
std::ostream& operator<< (std::ostream& strm, QuadGS::severity_level level)
{
    static const char* strings[] =
    {
        "error",
        "warning",
        "info",
        "debug",
        "message_trace"
    };

    if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast< int >(level);

    return strm;
}


void Log::Init( std::string FilenameAppLog, std::string FilenameMsgLog, std::ostream& outstream, severity_level svl )
{
    // Create the formatter for all file sinks.
    logging::formatter fmt = expr::stream
             << "[" << expr::attr< unsigned int >("RecordID") // First an attribute "RecordID" is written to the log
             << "] [" << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S.%f")
             << expr::if_(expr::has_attr("Severity"))
                   [
                    expr::stream << "] ["
                    << expr::attr< QuadGS::severity_level >("Severity")
                   ]
             << "] " // then this delimiter separates it from the rest of the line
             << expr::if_(expr::has_attr("Tag"))
                [
                    expr::stream << "[" << expr::attr< std::string >("Tag")
                     << "] " // yet another delimiter
                ]
             << expr::smessage; // here goes the log record text

    // Create the formatter for the clog sink.
    logging::formatter fmt_clog = expr::stream
             << expr::if_(expr::has_attr("Severity"))
                   [
                    expr::stream << "["
                    << expr::attr< QuadGS::severity_level >("Severity")
                    << "] "
                   ]
             << expr::if_(expr::has_attr("Tag"))
                [
                    expr::stream << "[" << expr::attr< std::string >("Tag")
                     << "] "
                ]
             << expr::smessage; // here goes the log record text

    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;


    // The file sink that will be used to log app messages.
    shared_ptr< text_sink > pSink = boost::make_shared< text_sink >();
    pSink->locked_backend()->add_stream(
            boost::make_shared< std::ofstream >(FilenameAppLog + ".log"));
    // The same formatter is used for all log entries.
    pSink->set_formatter(fmt);
    pSink->locked_backend()->auto_flush(true);
    logging::core::get()->add_sink(pSink);

    // Create the sink for message traces.
    pSink = boost::make_shared< text_sink >();
    pSink->locked_backend()->add_stream(
        boost::make_shared< std::ofstream >(FilenameMsgLog + ".log"));
    pSink->set_formatter(fmt);
    // The message trase sink should only log messages of type message_trace.
    pSink->set_filter(expr::attr< severity_level >("Severity") == message_trace );
    pSink->locked_backend()->auto_flush(true);
    logging::core::get()->add_sink(pSink);

    pSink = boost::make_shared< text_sink >();
    boost::shared_ptr< std::ostream > stream(&outstream, boost::null_deleter());
    pSink->locked_backend()->add_stream(stream);
    pSink->set_formatter(fmt_clog);
    pSink->set_filter(expr::attr< severity_level >("Severity") <= svl );
    logging::core::get()->add_sink(pSink);


    attrs::counter< unsigned int > RecordID(0);
    logging::core::get()->add_global_attribute("RecordID", RecordID);

    attrs::local_clock TimeStamp;
    logging::core::get()->add_global_attribute("TimeStamp", TimeStamp);

}
} /* namespace QuadGS */
