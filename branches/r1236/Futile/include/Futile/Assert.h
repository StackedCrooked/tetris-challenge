#ifndef ASSERT_H
#define ASSERT_H


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


#endif // ASSERT_H
