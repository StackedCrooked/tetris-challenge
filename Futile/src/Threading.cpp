#include "Futile/Threading.h"
#include "Futile/Atomic.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <ctime>


namespace Futile {


void Sleep(UInt64 inMilliseconds)
{
    boost::this_thread::sleep(boost::posix_time::milliseconds(inMilliseconds));
}


UInt64 GetCurrentTimeMs()
{
    boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration(time.time_of_day());
    return static_cast<UInt64>(duration.total_milliseconds());
}


namespace {

Atomic<unsigned> gMaxDuration(500);


unsigned GetMaxDuration()
{
    unsigned result = gMaxDuration.get();
    if (result % 10 == 0)
    {
        LogInfo(SS() << "Max duration is now: " << result << " ms.");
    }
    gMaxDuration.decrement(1);
    return result;
}


}


LifeTimeChecker::LifeTimeChecker() :
	mBeginTime(GetCurrentTimeMs()),
	mMaxDuration(100)
{
}


LifeTimeChecker::~LifeTimeChecker()
{
	Assert(GetCurrentTimeMs() - mBeginTime <= mMaxDuration);
}



} // namespace Futile
