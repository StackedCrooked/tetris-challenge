#include "Futile/Timer.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include <iostream>
#include <stdexcept>


namespace Futile {


struct Timer::Impl : boost::noncopyable
{
    Impl(Timer * inTimer,
         UInt64 inInterval) :
        mTimer(inTimer),
        mMainWorker("Timer"),
        mAction(),
        mStopMutex(),
        mStop(false),
        mInterval(inInterval)
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

    void start(const Action & inAction)
    {
        if (mMainWorker.status() != WorkerStatus_Waiting)
        {
            throw std::logic_error("Timer is busy.");
        }

        mAction = inAction;
        mMainWorker.schedule(boost::bind(&Impl::poll, this));
    }

    void stop()
    {
        setStopped();
        mMainWorker.interruptAndClearQueue();
        mMainWorker.wait();
    }

    void setInterval(UInt64 inInterval)
    {
        ScopedLock lock(mIntervalMutex);
        mInterval = inInterval;
    }

    UInt64 getInterval()
    {
        ScopedLock lock(mIntervalMutex);
        return mInterval;
    }

    void poll()
    {
        UInt64 startTime = GetCurrentTimeMs();
        while (!isStopped())
        {
            UInt64 currentTimeMs = GetCurrentTimeMs();
            if (currentTimeMs - startTime >= getInterval())
            {
                invokeCallback();
                startTime = currentTimeMs;
            }
            Sleep(2);
        }
    }

    void invokeCallback()
    {
        try
        {
            if (mAction)
            {
                mAction();
            }
        }
        catch (const std::exception & exc)
        {
            LogError(SS() << "Timer callback throws: " << exc.what());
        }
    }

    Timer * mTimer;
    Worker mMainWorker;
    Action mAction;

    mutable Mutex mIntervalMutex;
    UInt64 mInterval;

    mutable Mutex mStopMutex;
    bool mStop;
};


Timer::Timer(UInt64 inInterval) :
    mImpl(new Impl(this, inInterval))
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


void Timer::start(const Action & inAction)
{
    mImpl->start(inAction);
}


void Timer::stop()
{
    mImpl->stop();
}


void Timer::setInterval(UInt64 inInterval)
{
    mImpl->setInterval(inInterval);
}


} // namespace Futile
