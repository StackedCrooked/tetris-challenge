#ifndef ERROR_HANDLING_INCLUDED
#define ERROR_HANDLING_INCLUDED


#include "Poco/Debugger.h"
#include <stdexcept>
#include <string>


namespace Tetris
{

    #define Assert(condition) if (!(condition)) { Poco::Debugger::enter(__FILE__, __LINE__); }

} // namespace Tetris


#endif // ERROR_HANDLING_INCLUDED
