#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED


#include <boost/function.hpp>
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

    class Logger
    {
    public:
        static Logger & Instance();

        typedef boost::function<void(const std::string &)> Handler;

        void setHandler(const Handler & inHandler);

        void log(LogLevel inLogLevel, const std::string & inMessage);

    private:
        Handler mHandler;
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
