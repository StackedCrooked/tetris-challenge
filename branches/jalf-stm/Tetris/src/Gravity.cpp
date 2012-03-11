#include "Poco/Foundation.h"
#include "Tetris/Gravity.h"
#include "Tetris/Direction.h"
#include "Tetris/Game.h"
#include "Tetris/GameState.h"
#include "Tetris/GameStateNode.h"
#include "Futile/Assert.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Stopwatch.h"
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


const int cIntervalCount = sizeof(sIntervals)/sizeof(int);
extern const int cMaxLevel = sizeof(sIntervals)/sizeof(int) - 1;


struct Gravity::Impl : boost::noncopyable
{
    Impl(Gravity & inGravity, Game & inGame) :
        mGravity(inGravity),
        mGame(inGame),
        mLevel(0)
    {
    }

    ~Impl()
    {
    }

    void onTimerEvent();

    Gravity & mGravity;
    Game & mGame;
    stm::shared<int> mLevel;
};


Gravity::Gravity(Game & inGame) :
    mImpl(new Impl(*this, inGame)),
    mTimer()
{
    mTimer.reset(new Timer(sIntervals[std::max(inGame.level(), cMaxLevel)]));
    mTimer->start(boost::bind(&Gravity::Impl::onTimerEvent, mImpl.get()));
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


void Gravity::Impl::onTimerEvent()
{
    try
    {
        int newLevel = -1;
        // If our block was "caught" by the sudden appearance of new blocks, then we solidify it in that state.
        stm::atomic([&](stm::transaction & tx) {
            const Block & block = mGame.activeBlock(tx);
            if (!mGame.gameState(tx).checkPositionValid(block))
            {
                 mGame.move(tx, MoveDirection_Down);
                 return;
            }

            if (mGame.isGameOver(tx) || mGame.isPaused(tx))
            {
                return;
            }

            mGame.move(tx, MoveDirection_Down);
            int & level = mLevel.open_rw(tx);
            level = std::max(mGame.level(tx), cMaxLevel);
            newLevel = level;
        });

        if (newLevel != -1)
        {
            Assert(newLevel < cIntervalCount);
            mGravity.mTimer->setInterval(sIntervals[newLevel]);
        }
    }
    catch (const std::exception & inException)
    {
        LogError(inException.what());
    }
}


} // namespace Tetris
