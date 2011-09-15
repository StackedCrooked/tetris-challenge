#include "Futile/Stopwatch.h"
#include "Futile/Threading.h"
#include <stdexcept>
#include <iostream>


namespace Futile {


Stopwatch::Stopwatch() :
    mStart(GetCurrentTimeMs())
{
}


void Stopwatch::restart()
{
    mStart = GetCurrentTimeMs();
}


UInt64 Stopwatch::elapsedMs() const
{
    return static_cast<UInt64>(0.5 + 1.0 * (GetCurrentTimeMs() - mStart));
}


} // namespace Futile
