#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED


#include "Tetris/Threading.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <sstream>
#include <string>


namespace Tetris
{

    enum LogLevel
    {
        LogLevel_Info,
        LogLevel_Warning,
        LogLevel_Error
    };

    std::string ToString(LogLevel inLogLevel);

    // Requires that the ToString function is defined for values of type T.
    template<class T>
    std::string ToString(const std::vector<T> & inVector)
    {
        std::stringstream result;
        for (size_t idx = 0; idx != inVector.size(); ++idx)
        {
            if (idx != 0)
            {
                result << ", ";
            }
            result << ToString(inVector[idx]);            
        }
        return result.str();
    }

    class Logger
    {
    public:
        static Logger & Instance();

        typedef boost::function<void(const std::string &)> Handler;

        void setHandler(const Handler & inHandler);

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

        Handler mHandler;
        typedef std::vector<std::string> Queue;
        Protected<Queue> mProtectedQueue;
        boost::mutex mQueueMutex;
    };

    void LogInfo(const std::string & inMessage);

    void LogWarning(const std::string & inMessage);

    void LogError(const std::string & inMessage);

    /**
     * MakeString
     *
     * Enables logging like this:
     *
     *   LogError(MakeString() << "The index " << i << " exceeds the max length of " << max << ".");
     *
     */
    class MakeString
    {
    public:
       template <typename T>
       MakeString& operator<<(const T & datum)
       {
          mBuffer << datum;
          return *this;
       }
       operator std::string () const
       {
          return mBuffer.str();
       }
    private:
       std::ostringstream mBuffer;
    };

} // namespace Tetris


#endif // LOGGER_H_INCLUDED
