#include "Futile/Threading.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <ctime>


namespace Futile {


void Sleep(UInt64 inMilliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(inMilliseconds));
}


UInt64 GetCurrentTimeMs()
{
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration(time.time_of_day());
    return static_cast<UInt64>(duration.total_milliseconds());
}


namespace {


unsigned GetMaxDuration()
{
    static unsigned fMaxDuration = 1000;
    if (fMaxDuration == 1)
    {
        fMaxDuration = 1000;
    }
    fMaxDuration -= 1;
    Assert(fMaxDuration >= 1);
    return fMaxDuration;
}


} // anonymous namespace


LifeTimeChecker::LifeTimeChecker() :
	mBeginTime(GetCurrentTimeMs()),
    mMaxDuration(GetMaxDuration())
{
}


LifeTimeChecker::~LifeTimeChecker()
{
    UInt64 duration = (GetCurrentTimeMs() - mBeginTime);
    if (duration > mMaxDuration)
    {
        LogWarning(SS() << "Lock was held for " << duration << " ms");
    }
}



} // namespace Futile
