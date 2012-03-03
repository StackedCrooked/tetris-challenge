#include "Futile/Logger.h"
#include "Futile/Threading.h"
#include "stm.hpp"
#include <algorithm>
#include <utility>


namespace Futile {


std::string ConvertLogLevelToString(LogLevel inLogLevel)
{
    switch (inLogLevel)
    {
        case LogLevel_Info:
        {
            return "INFO";
        }
        case LogLevel_Warning:
        {
            return "WARNING";
        }
        case LogLevel_Error:
        {
            return "ERROR";
        }
        default:
        {
            throw std::invalid_argument("Invalid LogLevel enum value");
        }
    }
}


namespace {


typedef std::vector< std::pair<LogLevel, std::string> > MessageList;
typedef stm::shared< MessageList > SharedMessageList;
typedef Logger::LogHandler LogHandler;
typedef std::vector< std::pair<LogLevel, LogHandler> > LogHandlers;


}


struct Logger::Impl
{
    Impl() :
        mSharedMessageList(MessageList()),
        mLogHandlerMutex(),
        mLogHandlers()
    {
    }

    ~Impl()
    {
    }

    SharedMessageList mSharedMessageList;
    Mutex mLogHandlerMutex;
    LogHandlers mLogHandlers;
};


Logger::Logger() :
    mImpl(new Impl)
{
}


Logger::~Logger()
{
    mImpl.reset();
}


void Logger::addLogHandler(const LogHandler & inHandler)
{
    addLogHandler(LogLevel(-1), inHandler);
}


void Logger::addLogHandler(LogLevel inLogLevel, const LogHandler & inHandler)
{
    ScopedLock lock(mImpl->mLogHandlerMutex);
    mImpl->mLogHandlers.push_back(std::make_pair(inLogLevel, inHandler));
}


void Logger::flush()
{
    // Don't perform the logging (slow IO operations) inside the transaction.
    // Use a local copy instead.
    MessageList messages = stm::atomic<MessageList>([this](stm::transaction & tx) {
        MessageList & messages = mImpl->mSharedMessageList.open_rw(tx);
        MessageList copy = messages;
        messages.clear();
        return copy;
    });


    // Flush the logs.
    for (auto level_and_handler : mImpl->mLogHandlers)
    {
        auto level = level_and_handler.first;
        auto handler = level_and_handler.second;
        for (auto level_and_message : messages)
        {
            if (level == level_and_message.first)
            {
                handler(level_and_message.second);
            }
        }
    }
}


void Logger::log(LogLevel inLogLevel, const std::string & inMessage)
{
    stm::atomic([&](stm::transaction & tx) {
        MessageList & messages = mImpl->mSharedMessageList.open_rw(tx);
        messages.push_back(std::make_pair(inLogLevel, inMessage));
    });
}


} // namespace Futile
