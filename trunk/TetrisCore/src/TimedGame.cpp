#include "TimedGame.h"
#include "Game.h"


namespace Tetris
{

    // Number of milliseconds between two drops.
    static const int sIntervals[] = {
        887, 820, 753, 686, 619,
        552, 469, 368, 285, 184,
        167, 151, 134, 117, 100,
        100, 84, 84, 67, 67, 50
    };

    static const int sIntervalCount = sizeof(sIntervals)/sizeof(int);

    TimedGame::TimedGame(const Protected<Game> & inThreadSafeGame) :
        mThreadSafeGame(inThreadSafeGame),
        mLevel(0)
    {
    }


    void TimedGame::start()
    {
        mTimer.start(Poco::TimerCallback<TimedGame>(*this, &TimedGame::onTimerEvent));        
    }


    void TimedGame::onTimerEvent(Poco::Timer & inTimer)
    {
        // Scoped lock
        {
            ScopedAtom<Game> game(mThreadSafeGame);
            if (!game->isGameOver())
            {
                game->move(Direction_Down);
            }
        }
        mTimer.setPeriodicInterval(sIntervals[mLevel]);
    }


    int TimedGame::level() const
    {
        return mLevel;
    }


    void TimedGame::setLevel(int inLevel)
    {
        mLevel = std::min<int>(inLevel, sIntervalCount - 1);
    }


} // namespace Tetris
