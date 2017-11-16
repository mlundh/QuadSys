/*
 * LogHandler.h
 *
 *  Created on: Nov 18, 2017
 *      Author: martin
 */

#ifndef CORE_LOGHANDLER_LOGHANDLER_H_
#define CORE_LOGHANDLER_LOGHANDLER_H_

#include <functional>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "QuadLogMsg.h"

#define NR_LOG_FIELDS (3)
namespace QuadGS {

class QCMsgHeader;
class Command;
class QuadLogMsg;
class QuadGSMsg;
class LogHandler: public std::enable_shared_from_this<LogHandler>
{
    typedef std::function<void(std::shared_ptr<QCMsgHeader>, std::shared_ptr<QuadGSMsg>) > WriteFcn;
    typedef std::shared_ptr<LogHandler> ptr;
    /**
     * Private constructor, use create instead.
     */
    LogHandler();

public:

    virtual ~LogHandler();

    /**
     * Create a LogHandler object.
     * @return  Shared ptr to the object.
     */
    static ptr create();

    /**
     * Register the function that will write the packet to the client.
     * @param fcn   write function.
     */
    void RegisterWriteFcn(WriteFcn fcn);

    /**
     * Get the user commands this module supports.
     * @return
     */
    std::vector<std::shared_ptr<Command> > getCommands();

    /**
     * Get the name id mapping of all loggers.
     * @param
     * @return
     */
    std::string getLogNames(std::string );

    /**
     * Get the log entries.
     * @param
     * @return
     */
    std::string getLogEntries(std::string );

    /**
     * Stop all logging.
     * @param
     * @return
     */
    std::string stopAllLogs(std::string );

    /**
     * Format the log mapping.
     * @param string
     * @return           string containing the formatted log mapping.
     */
    std::string FormatLogMapping(std::string string);

    /**
     * Handle the incoming log messages.
     * @param header    Header of the message.
     * @param payload   Payload of the message.
     */
    void Handler(std::shared_ptr<QCMsgHeader> header, std::shared_ptr<QuadLogMsg> payload);

private:
    std::ofstream mLogFile;
    std::ofstream mMapFile;
    std::map<std::string, int> mNames;
    WriteFcn mWriteFcn;

};

} /* namespace QuadGS */

#endif /* CORE_LOGHANDLER_LOGHANDLER_H_ */
