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


void Logger::setLogHandler(const LogHandler & inHandler)
{
    mHandler = inHandler;
}


void Logger::flush()
{
    std::vector<std::string> items;

    // We don't want to perform the logging during the lock.
    // Therefore we make a local copy of the messages.
    FUTILE_LOCK(MessageList & messageList, mMessageList)
    {
        items = messageList;
        messageList.clear();
    }

    // Do the actual logging.
    if (mHandler)
    {
        std::for_each(items.begin(), items.end(), mHandler);
    }
}


void Logger::log(LogLevel inLogLevel, const std::string & inMessage)
{
    mMessageList.lock()->push_back(GetMessage(inLogLevel, inMessage));
}


} // namespace Futile

