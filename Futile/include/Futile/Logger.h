#ifndef LOGGER_H
#define LOGGER_H


#include "Futile/LeakDetector.h"
#include "Futile/Threading.h"
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

    void setLogHandler(const LogHandler& inHandler);

    void log(LogLevel inLogLevel, const std::string& inMessage);

    // MessageList posted from worker threads are stored in a queue.
    // The actual logging is delayed until:
    //   - a log message is posted from the main thread
    //   - flush() is called
    //
    // This method should probably only be called from the main thread.
    void flush();

protected:
    Logger() {}

    ~Logger() {}

private:
    friend class Singleton<Logger>;

    void logImpl(const std::string& inMessage);

    LogHandler mHandler;
    typedef std::vector<std::string> MessageList;
    ThreadSafe<MessageList> mMessageList;
};


} // namespace Futile


#endif // LOGGER_H
