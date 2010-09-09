#ifndef ERROR_HANDLING_INCLUDED
#define ERROR_HANDLING_INCLUDED


#ifdef _DEBUG
#include <windows.h> // required for the ::DebugBreak function
#else
#ifndef NDEBUG
#include <sstream>
#include <stdexcept>
#endif
#endif


#include <stdexcept>
#include <string>


#ifdef _DEBUG
#define Assert(condition) if (!(condition)) { ::DebugBreak(); }
#elif NDEBUG
#define Assert(...)
#else
#define Assert(condition) \
    if (!(condition) \
    { \
        std::stringstream ss; \
        throw std::logic_error(ss << __FILE__ << ":" << __LINE__); \
    }
#endif


#endif // ERROR_HANDLING_INCLUDED
