#ifndef ERROR_HANDLING_INCLUDED
#define ERROR_HANDLING_INCLUDED


#ifdef _DEBUG
    #ifdef _WIN32
        #include <windows.h>
        #define Assert(condition) if (!(condition)) { ::DebugBreak(); }
    #else
        #include "Tetris/MakeString.h"
        #include <stdexcept>
        #define Assert(condition) if (!(condition)) { throw std::logic_error(Tetris::MakeString() << __FILE__ << __LINE__ << ": Assert failed."); }
    #endif
#else
    #define Assert(...)
#endif


#endif // ERROR_HANDLING_INCLUDED
