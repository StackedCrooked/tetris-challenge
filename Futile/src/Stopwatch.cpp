#include "Futile/Stopwatch.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <stdexcept>
#include <iostream>


namespace Futile {


boost::uint64_t Stopwatch::GetCurrentTimeMs()
{
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration( time.time_of_day() );
    return static_cast<unsigned long long>(duration.total_milliseconds());
}


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
