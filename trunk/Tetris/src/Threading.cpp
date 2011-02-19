#include "Tetris/Config.h"
#include "Tetris/Threading.h"
#include "Poco/Stopwatch.h"


namespace Tetris {


InstanceTracker::ThreadedInstances InstanceTracker::sInstances;
const int cMaximumLockDurationMs(32);


class StopwatchImpl
{
public:
    StopwatchImpl()
    {
        mStopwatch.start();
    }


    ~StopwatchImpl()
    {
    }


    int elapsedTimeMs() const
    {
        return static_cast<int>(0.5 + static_cast<double>(mStopwatch.elapsed()) / 1000.0);
    }

private:
    StopwatchImpl(const StopwatchImpl&);
    StopwatchImpl& operator=(const StopwatchImpl&);

    Poco::Stopwatch mStopwatch;
};


Stopwatch::Stopwatch() :
    mImpl(new StopwatchImpl)
{
}


Stopwatch::~Stopwatch()
{
    delete mImpl;
    mImpl = 0;
}


int Stopwatch::elapsedTimeMs() const
{
    return mImpl->elapsedTimeMs();
}


} // namespace Tetris
