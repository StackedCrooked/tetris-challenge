#include "Tetris/Logging.h"
#include "Tetris/Logger.h"


namespace Tetris
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

} // namespace Tetris
