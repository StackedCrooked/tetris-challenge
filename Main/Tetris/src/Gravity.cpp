#include "Poco/Foundation.h"
#include "Tetris/Config.h"
#include "Tetris/Gravity.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Game.h"
#include "Tetris/Direction.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Assert.h"
#include "Poco/Timer.h"
#include "Poco/Stopwatch.h"
#include <boost/noncopyable.hpp>
#include <algorithm>


using Futile::LogError;
using Futile::ScopedReader;
using Futile::ScopedWriter;


namespace Tetris {


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
    Impl(ThreadSafe<Game> inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame),
        mLevel(0)
    {
    }

    ~Impl()
    {
    }

    void onTimerEvent(Poco::Timer & timer);

    int interval() const
    {
        return static_cast<int>(0.5 + CalculateSpeed(mLevel) / 1000.0);
    }

    static double CalculateSpeed(int inLevel)
    {
        return static_cast<double>(1000.0 / static_cast<double>(sIntervals[inLevel]));
    }

    ThreadSafe<Game> mThreadSafeGame;
    int mLevel;
    Poco::Timer mTimer;
    Poco::Stopwatch mStopwatch;
};


Gravity::Gravity(const ThreadSafe<Game> & inThreadSafeGame) :
    mImpl(new Impl(inThreadSafeGame))
{
    ScopedReader<Game> rgame(mImpl->mThreadSafeGame);
    mImpl->mTimer.start(Poco::TimerCallback<Gravity::Impl>(*mImpl, &Gravity::Impl::onTimerEvent));
    mImpl->mTimer.setPeriodicInterval(sIntervals[rgame->level()]);
    mImpl->mStopwatch.start();
}


Gravity::~Gravity()
{
    mImpl.reset();
}


void Gravity::Impl::onTimerEvent(Poco::Timer & )
{
    try
    {
        int oldLevel = mLevel;
        if (mStopwatch.elapsed() > 1000  * interval())
        {
            mStopwatch.restart();
            ScopedWriter<Game> game(mThreadSafeGame);
            if (game->isGameOver() || game->isPaused())
            {
                return;
            }
            game->move(MoveDirection_Down);
            mLevel = game->level();
            if (mLevel > cMaxLevel)
            {
                mLevel = cMaxLevel;
            }
        }
        if (mLevel != oldLevel)
        {
            Assert(mLevel < cIntervalCount);
            mTimer.setPeriodicInterval(sIntervals[mLevel]);
        }
    }
    catch (const std::exception & inException)
    {
        LogError(inException.what());
    }
}


double Gravity::speed() const
{
    return CalculateSpeed(mImpl->mLevel);
}


double Gravity::CalculateSpeed(int inLevel)
{
    return Impl::CalculateSpeed(inLevel);
}


} // namespace Tetris