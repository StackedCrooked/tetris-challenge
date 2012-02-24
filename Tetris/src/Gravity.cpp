#include "Poco/Foundation.h"
#include "Tetris/Gravity.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Game.h"
#include "Tetris/Direction.h"
#include "Futile/Stopwatch.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Assert.h"
#include "Futile/Timer.h"
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


const std::size_t cIntervalCount = sizeof(sIntervals)/sizeof(int);
extern const std::size_t cMaxLevel = sizeof(sIntervals)/sizeof(int) - 1;


struct Gravity::Impl : boost::noncopyable
{
    Impl(Gravity * inGravity, Game & inGame) :
        mGravity(inGravity),
        mGame(inGame),
        mLevel(0),
        mStopwatch()
    {
        mStopwatch.start();
    }

    ~Impl()
    {
    }

    void onTimerEvent();

    std::size_t intervalMs() const
    {
        return static_cast<int>(0.5 + CalculateSpeed(mLevel) / 1000.0);
    }

    static double CalculateSpeed(int inLevel)
    {
        return static_cast<double>(1000.0 / static_cast<double>(sIntervals[inLevel]));
    }

    Gravity * mGravity;
    Game & mGame;
    std::size_t mLevel;
    Futile::Stopwatch mStopwatch;
};


Gravity::Gravity(Game & inGame) :
    mImpl(new Impl(this, inGame)),
    mTimer()
{
    stm::atomic([&](stm::transaction & tx){
        mTimer.reset(new Timer(sIntervals[inGame.level(tx)]));
        mTimer->start(boost::bind(&Gravity::onTimerEvent, this));
    });
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
        LogError(SS() << "~Gravity throws: " << exc.what());
    }
}


void Gravity::onTimerEvent()
{
    FUTILE_LOCK(Impl & impl, mImpl)
    {
        impl.onTimerEvent();
    }
}


void Gravity::Impl::onTimerEvent()
{
    try
    {
        std::size_t oldLevel = mLevel;
        if (mStopwatch.elapsedMs() > intervalMs())
        {
            mStopwatch.restart();
                // If our block was "caught" by the sudden appearance of new blocks, then we solidify it in that state.
            stm::atomic([&](stm::transaction & tx) {
                const Block & block = mGame.activeBlock(tx);
                if (!mGame.gameState(tx).checkPositionValid(block))
                {
                     mGame.move(tx, MoveDirection_Down);
                     return;
                }

                if (mGame.isGameOver() || mGame.isPaused())
                {
                    return;
                }

                mGame.move(tx, MoveDirection_Down);
                mLevel = mGame.level(tx);
                if (mLevel > cMaxLevel)
                {
                    mLevel = cMaxLevel;
                }
            });
        }
        if (mLevel != oldLevel)
        {
            Assert(mLevel < cIntervalCount);
            mGravity->mTimer->setInterval(sIntervals[mLevel]);
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
