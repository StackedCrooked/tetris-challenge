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


boost::uint64_t Stopwatch::elapsedMs() const
{
    return static_cast<unsigned>(0.5 + 10.0 * (GetCurrentTimeMs() - mStart));
}


} // namespace Futile
