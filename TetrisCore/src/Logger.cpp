#include "Logger.h"
#include <stdexcept>


namespace Tetris
{
    
    Logger & Logger::Instance()
    {
        static Logger fInstance;
        return fInstance;
    }
    
    
    void Logger::setHandler(const Handler & inHandler)
    {
        mHandler = inHandler;
    }


    void Logger::log(LogLevel inLogLevel, const std::string & inMessage)
    {
        if (mHandler)
        {
            mHandler(ToString(inLogLevel) + ": " + inMessage);
        }
    }


    std::string ToString(LogLevel inLogLevel)
    {
        switch (inLogLevel)
        {
            case LogLevel_Info:
            {
                return "Info";
            }
            case LogLevel_Warning:
            {
                return "Warning";
            }
            case LogLevel_Error:
            {
                return "Error";
            }
            default:
            {
                throw std::invalid_argument("Invalid LogLevel enum value");
            }
        }
    }


    void LogInfo(const std::string & inMessage)
    {
        Logger::Instance().log(LogLevel_Info, inMessage);
    }


    void LogWarning(const std::string & inMessage)
    {
        Logger::Instance().log(LogLevel_Warning, inMessage);
    }


    void LogError(const std::string & inMessage)
    {
        Logger::Instance().log(LogLevel_Error, inMessage);
    }

} // namespace Tetris