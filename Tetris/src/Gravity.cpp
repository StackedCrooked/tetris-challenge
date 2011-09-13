#include "Poco/Foundation.h"
#include "Tetris/Config.h"
#include "Tetris/Gravity.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/GameImpl.h"
#include "Tetris/Direction.h"
#include "Futile/Stopwatch.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Assert.h"
#include "Poco/Timer.h"
#include <boost/noncopyable.hpp>
#include <algorithm>


namespace Tetris {


using namespace Futile;


// Number of milliseconds between two drops.
static const int sIntervals[] =
{
    887, 820, 753, 686, 619,
    552, 469, 368, 285, 184,
    167, 151, 134, 117, 100,
    100, 84, 84, 67, 67, 50
};


const int cIntervalCount = sizeof(sIntervals)/sizeof(int);
extern const int cMaxLevel = sizeof(sIntervals)/sizeof(int) - 1;


struct Gravity::Impl : boost::noncopyable
{
    Impl(ThreadSafe<GameImpl> inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame),
        mLevel(0)
    {
    }

    ~Impl()
    {
    }

    void onTimerEvent(Poco::Timer & timer);

    int intervalMs() const
    {
        return static_cast<int>(0.5 + CalculateSpeed(mLevel) / 1000.0);
    }

    static double CalculateSpeed(int inLevel)
    {
        return static_cast<double>(1000.0 / static_cast<double>(sIntervals[inLevel]));
    }

    ThreadSafe<GameImpl> mThreadSafeGame;
    int mLevel;
    Futile::Stopwatch mStopwatch;
};


Gravity::Gravity(const ThreadSafe<GameImpl> & inThreadSafeGame) :
    mImpl(new Impl(inThreadSafeGame)),
    mTimer(new Poco::Timer)
{
    mTimer->start(Poco::TimerCallback<Gravity>(*this, &Gravity::onTimerEvent));
    mTimer->setPeriodicInterval(sIntervals[inThreadSafeGame.lock()->level()]);
}


Gravity::~Gravity()
{
    // Don't allow exceptions to escape from the destructor.
    try
    {
        // Stopping the timer ensures that the timer callback
        // has completed before destroying the Impl object.
        mTimer->stop();
        mImpl.reset();
    }
    catch (const std::exception & exc)
    {
        // Log any errors.
        LogError(MakeString() << "~Gravity throws: " << exc.what());
    }
}


void Gravity::onTimerEvent(Poco::Timer & timer)
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.onTimerEvent(timer);
    }
}


void Gravity::Impl::onTimerEvent(Poco::Timer & timer)
{
    try
    {
        int oldLevel = mLevel;
        if (mStopwatch.elapsedMs() > intervalMs())
        {
            mStopwatch.restart();
            Locker<GameImpl> wGame(mThreadSafeGame);
            if (wGame->isGameOver() || wGame->isPaused())
            {
                return;
            }

            wGame->move(MoveDirection_Down);
            mLevel = wGame->level();
            if (mLevel > cMaxLevel)
            {
                mLevel = cMaxLevel;
            }
        }
        if (mLevel != oldLevel)
        {
            Assert(mLevel < cIntervalCount);
            timer.setPeriodicInterval(sIntervals[mLevel]);
        }
    }
    catch (const std::exception & inException)
    {
        LogError(inException.what());
    }
}


double Gravity::speed() const
{
    return CalculateSpeed(mImpl.lock()->mLevel);
}


double Gravity::CalculateSpeed(int inLevel)
{
    return Impl::CalculateSpeed(inLevel);
}


} // namespace Tetris
