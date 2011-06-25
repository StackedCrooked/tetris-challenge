#ifndef ASSERT_H_INCLUDED
#define ASSERT_H_INCLUDED


#if defined (NDEBUG)
    #define Assert(...)
#else
    #ifdef _WIN32
        #include <windows.h>
        #define Assert(condition) if (!(condition)) { ::DebugBreak(); }
    #else
        #define Assert(condition) if (!(condition)) { __builtin_trap(); }
    #endif
#endif


#endif // ASSERT_H_INCLUDED
