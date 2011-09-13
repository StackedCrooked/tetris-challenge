#include "Futile/Stopwatch.h"
#include <stdexcept>


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
    static const clock_t CLOCKS_PER_MS = CLOCKS_PER_SEC / 1000;
    return static_cast<unsigned>((clock() - mStart) / CLOCKS_PER_MS);
}


} // namespace Futile
