#ifndef LOGGING_H
#define LOGGING_H


#include "Futile/MakeString.h"
#include <string>


namespace Futile {


void LogInfo(const std::string & inMessage);
void LogWarning(const std::string & inMessage);
void LogError(const std::string & inMessage);


#ifndef NDEBUG
#define LogDebug(msg) LogInfo("(debug)" + std::string(msg));
#else
#define LogDebug(...)
#endif


} // namespace Futile


#endif // LOGGING_H
