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
        mInterval(inInterval),
        mStopMutex(),
        mStop(false)
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
        if (mMainWorker.status() != WorkerStatus_Idle)
        {
            throw std::logic_error("Timer is busy.");
        }

        {
            ScopedLock lock(mMutex);
            mAction = inAction;
        }
        mMainWorker.schedule(boost::bind(&Impl::poll, this));
    }

    void stop()
    {
        setStopped();
        {
            ScopedLock lock(mMutex);
            mCondition.notify_one();
        }
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
        try
        {
            pollImpl();
        }
        catch (const boost::thread_interrupted &)
        {
            // OK
        }
        catch (const std::exception & exc)
        {
            LogError(SS() << "Timer poll throws: " << exc.what());
        }
    }

    void pollImpl()
    {
        while (!isStopped())
        {
            boost::system_time duration = boost::get_system_time() + boost::posix_time::milliseconds(getInterval());

            {
                ScopedLock lock(mMutex);
                if (mCondition.timed_wait(lock, duration))
                {
                    return;
                }
            }

            if (isStopped())
            {
                return;
            }


            {
                ScopedLock lock(mMutex);
                if (mAction)
                {
                    mAction();
                }
            }
        }
    }


    Timer * mTimer;

    Worker mMainWorker;

    Condition mCondition;
    Mutex mMutex;

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
