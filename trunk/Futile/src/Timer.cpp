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
         boost::uint64_t inStartInterval,
         boost::uint64_t inPeriodicInterval) :
        mTimer(inTimer),
        mMainWorker("Timer"),
        mAction(inAction),
        mStopMutex(),
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
        boost::uint64_t startTime = GetCurrentTimeMs();
        while (!isStopped())
        {
            if (GetCurrentTimeMs() - startTime >= mStartInterval)
            {
                invokeCallback();
                break;
            }
            Sleep(1);
        }

        // Periodic interval
        startTime = GetCurrentTimeMs();
        while (!isStopped())
        {
            if (GetCurrentTimeMs() - startTime >= mPeriodicInterval)
            {
                invokeCallback();
                startTime = GetCurrentTimeMs();
            }
            Sleep(1);
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
    boost::uint64_t mStartInterval;
    boost::uint64_t mPeriodicInterval;
    mutable Mutex mStopMutex;
    bool mStop;
};


Timer::Timer(const Action & inAction,
             boost::uint64_t inStartInterval,
             boost::uint64_t inPeriodicInterval) :
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
