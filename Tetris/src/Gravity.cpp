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
#include "Futile/STMSupport.h"
#include "Futile/Timer.h"
#include <boost/noncopyable.hpp>
#include <algorithm>


namespace Tetris {


using namespace Futile;


namespace { // anonymous


std::vector<int> GetIntervals()
{
    // Number of milliseconds between two drops.
    static const int sIntervals[] =
    {
        // Gameboy values
        887, 820, 753, 686, 619,
        552, 469, 368, 285, 184,
        167, 151, 134, 117, 100,
        100, 84, 84, 67, 67, 50,

        // Additional higher levels
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40,
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30,
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20,
        19, 18, 17, 16, 15
    };

    std::vector<int> result;
    for (unsigned i = 0; i < sizeof(sIntervals)/sizeof(sIntervals[0]); ++i)
    {
        result.push_back(sIntervals[i]);
    }
    return result;
}


static const std::vector<int> sIntervals = GetIntervals();
const int cMaxLevel = sIntervals.size() - 1;


int GetInterval(int inLevel)
{
    if (inLevel < 0)
    {
        return sIntervals.front();
    }
    else if (inLevel < int(sIntervals.size()))
    {
        return sIntervals[inLevel];
    }
    else
    {
        return sIntervals.back();
    }
}


} // anonymous namespace


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
    mTimer.reset(new Timer(GetInterval(inGame.level())));
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
        stm::atomic([this, &newLevel](stm::transaction & tx)
        {
            const Block & block = mGame.activeBlock();
            if (!mGame.checkPositionValid(block))
            {
                 mGame.move(MoveDirection_Down);
                 return;
            }

            if (mGame.isGameOver() || mGame.isPaused())
            {
                return;
            }

            mGame.move(MoveDirection_Down);
            int & level = mLevel.open_rw(tx);
            level = std::min(mGame.level(), cMaxLevel);
            newLevel = level;
        });

        if (newLevel != -1)
        {
            mGravity.mTimer->setInterval(GetInterval(newLevel));
        }
    }
    catch (const std::exception & inException)
    {
        LogError(inException.what());
    }
}


} // namespace Tetris
