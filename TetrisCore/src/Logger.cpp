#include "Logger.h"
#include "Poco/Thread.h"
#include <stdexcept>


namespace Tetris
{


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

    std::string GetMessage(LogLevel inLogLevel, const std::string & inMessage)
    {
        return ToString(inLogLevel) + ": " + inMessage;
    }


    Logger & Logger::Instance()
    {
        static Logger fInstance;
        return fInstance;
    }
    
    
    void Logger::setHandler(const Handler & inHandler)
    {
        mHandler = inHandler;
    }   
            

    void Logger::flush()
    {
        ScopedAtom<Queue> queue(mProtectedQueue);
        for (size_t idx = 0; idx != queue->size(); ++idx)
        {
            logImpl((*queue.get())[idx]);
        }
        queue->clear();
    }    


    void Logger::logImpl(const std::string & inMessage)
    {
        if (mHandler)
        {
            mHandler(inMessage);
        }
    }


    void Logger::log(LogLevel inLogLevel, const std::string & inMessage)
    {
        // Critical section: add the log message to the buffer
        {        
            ScopedAtom<Queue> queue(mProtectedQueue);
            queue->push_back(GetMessage(inLogLevel, inMessage));
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