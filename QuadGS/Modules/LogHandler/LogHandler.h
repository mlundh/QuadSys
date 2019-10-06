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

#include "QGS_Module.h"

#include "Msg_GetUiCommands.h"
#include "Msg_FireUiCommand.h"
#include "Msg_Log.h"

#define NR_LOG_FIELDS (3)

namespace QuadGS {

/**
 * Message format:
 * Name:
 * NameOfModule<valueType>[uniqueId]
 *
 * Entry:
 * [ID][TIME][DATA]
 *
 */
class LogHandler
		: public QGS_ReactiveModule
		, public QGS_MessageHandler<Msg_GetUiCommands>
		, public QGS_MessageHandler<Msg_FireUiCommand>
		, public QGS_MessageHandler<Msg_Log>

{
	typedef std::function<std::string(std::string) > UiFcn;
	class UiCommand
	{
	public:
		UiCommand(std::string command,  std::string doc, UiFcn function);
		std::string command;
		std::string doc;
		UiFcn function;
	};
public:

    LogHandler(msgAddr_t name, msgAddr_t loggerAddr);


    virtual ~LogHandler();


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

    std::string SetAppLogLevel(std::string string);


	virtual void process(Msg_GetUiCommands* message);
	virtual void process(Msg_FireUiCommand* message);
	virtual void process(Msg_Log* message);

    std::ofstream mLogFile;
    std::ofstream mMapFile;
    std::map<std::string, int> mNames;
	std::vector<UiCommand> mCommands;
    msgAddr_t mLoggerAddr;

};

} /* namespace QuadGS */

#endif /* CORE_LOGHANDLER_LOGHANDLER_H_ */
