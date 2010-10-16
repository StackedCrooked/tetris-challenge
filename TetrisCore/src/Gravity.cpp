#include "Tetris/Gravity.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Direction.h"
#include "Tetris/Logging.h"


namespace Tetris
{

    // Number of milliseconds between two drops.
    static const int sIntervals[] =
    {
        887, 820, 753, 686, 619,
        552, 469, 368, 285, 184,
        167, 151, 134, 117, 100,
        100, 84, 84, 67, 67, 50
    };

    static const int cMaxLevel = sizeof(sIntervals)/sizeof(int) - 1;

    Gravity::Gravity(const Protected<Game> & inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame),
        mLevel(0)
    {
        mTimer.start(Poco::TimerCallback<Gravity>(*this, &Gravity::onTimerEvent));
        mTimer.setPeriodicInterval(sIntervals[getLevel()]);
    }


    Gravity::~Gravity()
    {
        mTimer.stop();
    }


    void Gravity::onTimerEvent(Poco::Timer & inTimer)
    {
        try
        {
            int level = 0;
            // Scoped lock
            {
                ScopedAtom<Game> game(mThreadSafeGame, 1000);
                if (!game->isGameOver())
                {
                    game->move(Direction_Down);
                }
                level = game->currentNode()->state().stats().numLines() / 10;
            }

            if (level > getLevel() && level <= cMaxLevel)
            {
                setLevel(level);
                mTimer.setPeriodicInterval(sIntervals[level]);
            }
        }
        catch (const std::exception & inException)
        {
            LogError(inException.what());
        }
    }


    float Gravity::currentSpeed() const
    {
        return static_cast<float>(1000.0 / static_cast<float>(sIntervals[getLevel()]));
    }


    int Gravity::getLevel() const
    {
        boost::mutex::scoped_lock lock(mLevelMutex);
        return mLevel;
    }


    void Gravity::setLevel(int inLevel)
    {
        boost::mutex::scoped_lock lock(mLevelMutex);
        mLevel = std::min<int>(inLevel, cMaxLevel);
        mTimer.setPeriodicInterval(sIntervals[mLevel]);
    }


} // namespace Tetris
