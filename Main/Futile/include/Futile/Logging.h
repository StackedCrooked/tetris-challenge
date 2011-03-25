#ifndef FUTILE_LOGGING_H_INCLUDED
#define FUTILE_LOGGING_H_INCLUDED


#include "Futile/MakeString.h"
#include <string>


namespace Futile {


void LogInfo(const std::string & inMessage);

void LogWarning(const std::string & inMessage);

void LogError(const std::string & inMessage);


} // namespace Futile


#endif // FUTILE_LOGGING_H_INCLUDED
