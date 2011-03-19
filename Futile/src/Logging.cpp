#include "Futile/Config.h"
#include "Futile/Logging.h"
#include "Futile/Logger.h"


namespace Futile
{

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

} // namespace Futile
