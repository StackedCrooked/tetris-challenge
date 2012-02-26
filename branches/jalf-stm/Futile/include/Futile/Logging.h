#ifndef LOGGING_H
#define LOGGING_H


#include "Futile/MakeString.h"
#include <string>


namespace Futile {


void LogInfoImpl(const std::string & inMessage);
void LogWarningImpl(const std::string & inMessage);
void LogErrorImpl(const std::string & inMessage);


#define LogInfo(msg) LogInfoImpl(Futile::SS() << __FILE__ << ":" << __LINE__ << ": " << std::string(msg))

#define LogWarning(msg) LogWarningImpl(Futile::SS() << __FILE__ << ":" << __LINE__ << ": " << std::string(msg))

#define LogError(msg) LogErrorImpl(Futile::SS() << __FILE__ << ":" << __LINE__ << ": " << std::string(msg))


} // namespace Futile


#endif // LOGGING_H
