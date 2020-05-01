#ifndef LOGGING_H_INCLUDED
#define LOGGING_H_INCLUDED


#include "Futile/MakeString.h"
#include <string>


namespace Futile {


void LogInfo(const std::string& inMessage);

void LogWarning(const std::string& inMessage);

void LogError(const std::string& inMessage);


} // namespace Futile


#endif // LOGGING_H_INCLUDED
