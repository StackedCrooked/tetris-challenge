#include "Futile/Timer.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include <iostream>
#include <stdexcept>


namespace Futile {


struct Timer::Impl : boost::noncopyable
{
    Impl(Timer * inTimer,
         const Action & inAction,
         unsigned inStartInterval,
         unsigned inPeriodicInterval) :
        mTimer(inTimer),
        mMainWorker("Timer"),
        mAction(inAction),
        mStop(false),
        mStartInterval(inStartInterval),
        mPeriodicInterval(inPeriodicInterval)
    {
    }

    ~Impl()
    {
        stop();
    }

    bool isStopped() const
    {
        ScopedLock lock(mStopMutex);
        return mStop;
    }

    void setStopped()
    {
        ScopedLock lock(mStopMutex);
        mStop = true;
    }

    void start()
    {
        if (mMainWorker.status() == WorkerStatus_Waiting)
        {
            mMainWorker.schedule(boost::bind(&Impl::poll, this));
        }
    }

    void stop()
    {
        setStopped();
        mMainWorker.interruptAndClearQueue();
        mMainWorker.wait();
    }

    void poll()
    {
        if (!isStopped())
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(mStartInterval));
            invokeCallback();
        }

        // Periodic interval
        while (!isStopped())
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(mPeriodicInterval));
            invokeCallback();
        }
    }

    void invokeCallback()
    {
        try
        {
            mAction();
        }
        catch (const std::exception & exc)
        {
            LogError(SS() << "Timer callback throws: " << exc.what());
        }
    }

    Timer * mTimer;
    Worker mMainWorker;
    Action mAction;
    bool mStop;
    mutable Mutex mStopMutex;
    unsigned mStartInterval;
    unsigned mPeriodicInterval;
};


Timer::Timer(const Action & inAction,
             unsigned inStartInterval,
             unsigned inPeriodicInterval) :
    mImpl(new Impl(this, inAction, inStartInterval, inPeriodicInterval))
{
}


Timer::~Timer()
{
    try
    {
        mImpl->stop();
        mImpl.reset();
    }
    catch (const std::exception & exc)
    {
        LogError(SS() << "~Timer throws: " << exc.what());
    }
}


void Timer::start()
{
    mImpl->start();
}


void Timer::stop()
{
    mImpl->stop();
}


} // namespace Futile
