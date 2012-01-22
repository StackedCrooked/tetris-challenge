#include "Futile/Stopwatch.h"
#include "Futile/Threading.h"
#include <stdexcept>
#include <iostream>


namespace Futile {


Stopwatch::Stopwatch() :
    mStart(0),
    mStop(0)
{
}


void Stopwatch::start()
{
    if (mStart == 0)
    {
        mStart = GetCurrentTimeMs();
    }
}


void Stopwatch::stop()
{
    if (mStop == 0)
    {
        mStop = GetCurrentTimeMs();
    }
}


void Stopwatch::restart()
{
    mStart = GetCurrentTimeMs();
    mStop = 0;
}


void Stopwatch::reset()
{
    mStart = 0;
    mStop = 0;
}


UInt64 Stopwatch::elapsedMs() const
{
    if (mStart != 0)
    {
        if (mStop != 0)
        {
            return mStop - mStart;
        }
        else
        {
            return GetCurrentTimeMs() - mStart;
        }
    }
    else
    {
        return 0;
    }

}


} // namespace Futile
