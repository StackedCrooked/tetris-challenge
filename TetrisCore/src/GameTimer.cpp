#include "GameTimer.h"


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

    GameTimer::GameTimer(Game * inGame) :
        mGame(inGame),
        mLevel(0)
    {
    }


    void GameTimer::start()
    {
        mTimer.start(Poco::TimerCallback<GameTimer>(*this, &GameTimer::onTimerEvent));        
    }


    void GameTimer::onTimerEvent(Poco::Timer & inTimer)
    {
        if (mGame->isGameOver())
        {
            mGame->move(Direction_Down);
        }

        mTimer.setPeriodicInterval(sIntervals[mLevel]);
    }


    int GameTimer::level() const
    {
        return mLevel;
    }


    void GameTimer::setLevel(int inLevel)
    {
        mLevel = std::min<int>(inLevel, sIntervalCount - 1);
    }


} // namespace Tetris
