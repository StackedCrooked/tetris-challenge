#include "Futile/Timer.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include <iostream>
#include <stdexcept>


namespace Futile {


struct Timer::Impl : boost::noncopyable
{
    Impl(Timer * inTimer,
         boost::uint64_t inStartInterval,
         boost::uint64_t inPeriodicInterval) :
        mTimer(inTimer),
        mMainWorker("Timer"),
        mAction(),
        mStopMutex(),
        mStop(false),
        mStartInterval(inStartInterval),
        mPeriodicInterval_(inPeriodicInterval)
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

    void setPeriodicInterval(boost::uint64_t inPeriodicInterval)
    {
        ScopedLock lock(mPeriodicIntervalMutex);
        mPeriodicInterval_ = inPeriodicInterval;
    }

    boost::uint64_t getPeriodicInterval()
    {
        ScopedLock lock(mPeriodicIntervalMutex);
        return mPeriodicInterval_;
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
            if (GetCurrentTimeMs() - startTime >= getPeriodicInterval())
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

    mutable Mutex mPeriodicIntervalMutex;
    boost::uint64_t mPeriodicInterval_;

    mutable Mutex mStopMutex;
    bool mStop;
};


Timer::Timer(boost::uint64_t inStartInterval,
             boost::uint64_t inPeriodicInterval) :
    mImpl(new Impl(this, inStartInterval, inPeriodicInterval))
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


void Timer::setPeriodicInterval(boost::uint64_t inPeriodicInterval)
{
    mImpl->setPeriodicInterval(inPeriodicInterval);
}


} // namespace Futile
