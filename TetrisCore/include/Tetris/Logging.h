#ifndef TETRIS_LOGGING_H_INCLUDED
#define TETRIS_LOGGING_H_INCLUDED


#include <string>


namespace Tetris
{

    void LogInfo(const std::string & inMessage);

    void LogWarning(const std::string & inMessage);

    void LogError(const std::string & inMessage);

} // namespace Tetris


#endif // LOGGING_H_INCLUDED
