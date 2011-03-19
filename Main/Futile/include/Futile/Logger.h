#ifndef FUTILE_LOGGER_H_INCLUDED
#define FUTILE_LOGGER_H_INCLUDED


#include "Futile/Threading.h"
#include <boost/function.hpp>
#include <string>
#include <vector>


namespace Futile
{

    enum LogLevel
    {
        LogLevel_Info,
        LogLevel_Warning,
        LogLevel_Error
    };

    std::string ToString(LogLevel inLogLevel);

    class Logger
    {
    public:
        static Logger & Instance();

        typedef boost::function<void(const std::string &)> LogHandler;

        void setLogHandler(const LogHandler & inHandler);

        void log(LogLevel inLogLevel, const std::string & inMessage);

        // Messages posted from worker threads are stored in a queue.
        // The actual logging is delayed until:
        //   - a log message is posted from the main thread
        //   - flush() is called
        //
        // This method should probably only be called from the main thread.
        void flush();

    private:
        void logImpl(const std::string & inMessage);

        LogHandler mHandler;
        typedef std::vector<std::string> Queue;
        ThreadSafe<Queue> mProtectedQueue;
    };

} // namespace Futile


#endif // LOGGER_H_INCLUDED
