#ifndef FUTILE_ASSERT_H_INCLUDED
#define FUTILE_ASSERT_H_INCLUDED


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


#endif // FUTILE_ASSERT_H_INCLUDED
