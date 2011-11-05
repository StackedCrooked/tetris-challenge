#ifndef LOGGER_H
#define LOGGER_H


#include "Futile/LeakDetector.h"
#include "stm.hpp"
#include <boost/function.hpp>
#include <string>
#include <vector>


namespace Futile {


enum LogLevel
{
    LogLevel_Info,
    LogLevel_Warning,
    LogLevel_Error
};


std::string ConvertLogLevelToString(LogLevel inLogLevel);


class Logger : public Singleton<Logger>
{
public:
    typedef boost::function<void(const std::string &)> LogHandler;

    void setLogHandler(const LogHandler & inHandler);

    void log(LogLevel inLogLevel, const std::string & inMessage);

    // Messages posted from worker threads are stored in a queue.
    // The actual logging is delayed until flush() is called.
    void flush();

protected:
    Logger();

    ~Logger() {}

private:
    friend class Singleton<Logger>;

    void logImpl(const std::string & inMessage);

    typedef std::vector<std::string> MessageList;
    typedef stm::shared<MessageList> SharedMessageList;
    SharedMessageList mSharedMessageList;
    LogHandler mLogHandler;
};


} // namespace Futile


#endif // LOGGER_H
