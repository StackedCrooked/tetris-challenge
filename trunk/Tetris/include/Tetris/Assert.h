#ifndef TETRIS_ASSERT_H_INCLUDED
#define TETRIS_ASSERT_H_INCLUDED


#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include <stdexcept>
#include <string>


namespace Tetris {


void DebugBreak(const std::string & inFile, int inLine);


} // namespace Tetris


#if not defined(NDEBUG) || defined(TETRIS_ALWAYS_ASSERT)
    #ifdef _WIN32
        #include <windows.h>
        #define Assert(condition) if (!(condition)) { ::DebugBreak(); }
    #else
        #include "Tetris/MakeString.h"
        #include <stdexcept>
        #define Assert(condition) if (!(condition)) { Tetris::DebugBreak(__FILE__, __LINE__); }
    #endif
#else
    #define Assert(...)
#endif


#endif // TETRIS_ASSERT_H_INCLUDED
