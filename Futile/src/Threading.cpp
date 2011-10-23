#include "Futile/Config.h"
#include "Futile/Threading.h"
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
    boost::posix_time::time_duration duration( time.time_of_day() );
    return static_cast<unsigned long long>(duration.total_milliseconds());
}


} // namespace Futile
