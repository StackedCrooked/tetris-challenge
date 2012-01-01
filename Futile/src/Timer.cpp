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
        mIntervalMutex(),
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
        boost::mutex::scoped_lock lock(mStopMutex);
        return mStop;
    }

    void setStopped()
    {
        boost::mutex::scoped_lock lock(mStopMutex);
        mStop = true;
    }

    void start(const Action & inAction)
    {
        if (mMainWorker.status() != WorkerStatus_Idle)
        {
            throw std::logic_error("Timer is busy.");
        }

        {
            boost::mutex::scoped_lock lock(mMutex);
            mAction = inAction;
        }
        mMainWorker.schedule(boost::bind(&Impl::poll, this));
    }

    void stop()
    {
        setStopped();
        {
            boost::mutex::scoped_lock lock(mMutex);
            mCondition.notify_one();
        }
        mMainWorker.interruptAndClearQueue();
        mMainWorker.wait();
    }

    void setInterval(UInt64 inInterval)
    {
        boost::mutex::scoped_lock lock(mIntervalMutex);
        mInterval = inInterval;
    }

    UInt64 getInterval() const
    {
        boost::mutex::scoped_lock lock(mIntervalMutex);
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
                boost::mutex::scoped_lock lock(mMutex);
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
                boost::mutex::scoped_lock lock(mMutex);
                if (mAction)
                {
                    mAction();
                }
            }
        }
    }


    Timer * mTimer;

    Worker mMainWorker;

    boost::condition_variable mCondition;
    boost::mutex mMutex;

    Action mAction;

    mutable boost::mutex mIntervalMutex;
    UInt64 mInterval;

    mutable boost::mutex mStopMutex;
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


UInt64 Timer::getInterval() const
{
    return mImpl->mInterval;
}


} // namespace Futile
