#include "Futile/Logging.h"
#include "Futile/Logger.h"
#include "Futile/MakeString.h"


namespace Futile {


void LogInfoImpl(const std::string & inMessage)
{
    Logger::Instance().log(LogLevel_Info, inMessage);
}


void LogWarningImpl(const std::string & inMessage)
{
    Logger::Instance().log(LogLevel_Warning, inMessage);
}


void LogErrorImpl(const std::string & inMessage)
{
    Logger::Instance().log(LogLevel_Error, inMessage);
}


} // namespace Futile
