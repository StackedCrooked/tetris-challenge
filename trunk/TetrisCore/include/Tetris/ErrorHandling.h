#ifndef ERROR_HANDLING_INCLUDED
#define ERROR_HANDLING_INCLUDED


#ifdef _DEBUG
    #include <windows.h>
    #define Assert(condition) if (!(condition)) { ::DebugBreak(); }
#else
    #define Assert(...)
#endif


#endif // ERROR_HANDLING_INCLUDED
