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
    // TODO Auto-generated constructor stub

}

Log::~Log()
{
    // TODO Auto-generated destructor stub
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
        "info",
        "warning",
        "error",
        "debug",
        "message_trace"
    };

    if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast< int >(level);

    return strm;
}

void Log::Init( std::string FilenameAppLog, std::string FilenameMsgLog )
{
    // Create the common formatter for all sinks.
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
                     << "]" // yet another delimiter
                ]
             << expr::smessage; // here goes the log record text

    // Create a text file sink
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;

    // Create a text file sink
    shared_ptr< text_sink > pSink = boost::make_shared< text_sink >();
    // Add a file stream to the sink.
    pSink->locked_backend()->add_stream(
            boost::make_shared< std::ofstream >(FilenameAppLog + ".log"));
    // Set formatter of the sink.
    pSink->set_formatter(fmt);
    // Add the sink to the core.
    logging::core::get()->add_sink(pSink);

    // Create a text file sink for the message trace.
    pSink = boost::make_shared< text_sink >();
    // Add a file stream to the sink.
    pSink->locked_backend()->add_stream(
        boost::make_shared< std::ofstream >(FilenameMsgLog + ".log"));
    // Set the formatter of the sink.
    pSink->set_formatter(fmt);
    // Set the filter of the stream.
    pSink->set_filter(expr::attr< severity_level >("Severity") == message_trace );
    // Add the stream to the core.
    logging::core::get()->add_sink(pSink);

    // Create a text file sink
    pSink = boost::make_shared< text_sink >();
    // Add a file stream to the sink.
    // We have to provide an empty deleter to avoid destroying the global stream object
    boost::shared_ptr< std::ostream > stream(&std::clog, boost::null_deleter());
    pSink->locked_backend()->add_stream(stream);
    // Set the formatter of the sink.
    pSink->set_formatter(fmt);
    // Add the stream to the core.
    logging::core::get()->add_sink(pSink);


    attrs::counter< unsigned int > RecordID(0);
    logging::core::get()->add_global_attribute("RecordID", RecordID);

    attrs::local_clock TimeStamp;
    logging::core::get()->add_global_attribute("TimeStamp", TimeStamp);

}
} /* namespace QuadGS */
