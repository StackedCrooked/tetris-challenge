#include "Futile/Stopwatch.h"
#include <stdexcept>
#include <iostream>


namespace Futile {


Stopwatch::Stopwatch() :
    mStart(clock())
{
}


void Stopwatch::restart()
{
    mStart = clock();
}


unsigned Stopwatch::elapsedMs() const
{
    return static_cast<unsigned>(0.5 + 10.0 * (clock() - mStart));
}


} // namespace Futile
