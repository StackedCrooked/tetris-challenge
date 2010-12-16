#ifndef TETRIS_ASSERT_H_INCLUDED
#define TETRIS_ASSERT_H_INCLUDED


#if defined(_DEBUG) || defined(TETRIS_ALWAYS_ASSERT)
    #ifdef _WIN32
        #include <windows.h>
        #define Assert(condition) if (!(condition)) { ::DebugBreak(); }
    #else
        #include "Tetris/MakeString.h"
        #include <stdexcept>
        #define Assert(condition) if (!(condition)) { throw std::logic_error(Tetris::MakeString() << __FILE__ << __LINE__ << ": Assert failed."); }
    #endif

    // Change assert(..) calls into Assert(...) calls.
    #ifdef assert
    #undef assert
    #define assert Assert
    #endif
#else
    #define Assert(...)
#endif


#endif // TETRIS_ASSERT_H_INCLUDED
