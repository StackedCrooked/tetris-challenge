#include "Futile/Logger.h"
#include <boost/bind.hpp>
#include <algorithm>
#include <stdexcept>


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


std::string GetMessage(LogLevel inLogLevel, const std::string & inMessage)
{
    return ConvertLogLevelToString(inLogLevel) + ": " + inMessage;
}


Logger::Logger() :
    mSharedMessageList(MessageList()),
    mLogHandlerMutex(),
    mLogHandlers()
{
}


void Logger::addLogHandler(const LogHandler & inHandler)
{
    ScopedLock lock(mLogHandlerMutex);
    mLogHandlers.push_back(inHandler);
}


void Logger::flush()
{
    MessageList messagesCopy;

    // Don't perform the logging (slow IO operations) inside the transaction.
    // Use a local copy instead.
    stm::atomic([&](stm::transaction & tx) {
        MessageList & messages = mSharedMessageList.open_rw(tx);
        messagesCopy = messages;
        messages.clear();
    });


    // Flush the logs.
    // Technically we may have a race condition on the mLogHandler here.

    ScopedLock lock(mLogHandlerMutex);
    for (auto handler : mLogHandlers)
    {
        for (auto message : messagesCopy)
        {
            handler(message);
        }
    }
}


void Logger::log(LogLevel inLogLevel, const std::string & inMessage)
{
    stm::atomic([&](stm::transaction & tx) {
        MessageList & messages = mSharedMessageList.open_rw(tx);
        messages.push_back(GetMessage(inLogLevel, inMessage));
    });
}


} // namespace Futile
