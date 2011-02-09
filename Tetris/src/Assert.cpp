#include "Tetris/Assert.h"


namespace Tetris {


void DebugBreak(const std::string & inFile, int inLine)
{
    std::string msg(MakeString() << inFile << ":" << inLine << " Assertion failed.");
    throw std::logic_error(msg.c_str());
}


} // namespace Tetris
