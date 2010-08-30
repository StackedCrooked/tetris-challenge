#ifndef ERROR_HANDLING_INCLUDED
#define ERROR_HANDLING_INCLUDED


#ifdef _DEBUG
#include "Poco/Debugger.h"
#else
#include <sstream>
#endif


#include <stdexcept>
#include <string>


#ifndef NDEBUG
#define Assert(condition) if (!(condition)) { Poco::Debugger::enter(__FILE__, __LINE__); }
#else
#define Assert(...)
#endif


#endif // ERROR_HANDLING_INCLUDED
