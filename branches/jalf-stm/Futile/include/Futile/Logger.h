#ifndef LOGGER_H
#define LOGGER_H


#include "Futile/Singleton.h"
#include <functional>
#include <memory>
#include <string>


namespace Futile {


enum LogLevel
{
    LogLevel_Debug,
    LogLevel_Info,
    LogLevel_Warning,
    LogLevel_Error
};


std::string ConvertLogLevelToString(LogLevel inLogLevel);


class Logger : public Singleton<Logger>
{
public:
    typedef std::function<void(const std::string &)> LogHandler;

    void addLogHandler(const LogHandler & inHandler);

    void addLogHandler(LogLevel level, const LogHandler & handler);

    void log(LogLevel inLogLevel, const std::string & inMessage);

    // Messages posted from worker threads are stored in a queue.
    // The actual logging is delayed until flush() is called.
    void flush();

private:
    friend class Singleton<Logger>;
    Logger();
    ~Logger();

    struct Impl;
    std::unique_ptr<Impl> mImpl;
};


} // namespace Futile


#endif // LOGGER_H
