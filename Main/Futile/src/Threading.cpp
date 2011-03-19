#include "Futile/Config.h"
#include "Futile/Threading.h"
#include "Poco/Stopwatch.h"


namespace Futile {


InstanceTracker::ThreadedInstances InstanceTracker::sInstances;


struct Stopwatch::Impl : boost::noncopyable
{
public:
    Impl()
    {
        mPocoStopwatch.start();
    }

    ~Impl()
    {
    }

    Poco::Stopwatch mPocoStopwatch;
};


Stopwatch::Stopwatch() :
    mImpl(new Impl)
{
}


Stopwatch::~Stopwatch()
{
    mImpl.reset();
}


int Stopwatch::elapsedTimeMs() const
{
    return static_cast<int>(0.5 + static_cast<double>(mImpl->mPocoStopwatch.elapsed()) / 1000.0);
}


} // namespace Futile
