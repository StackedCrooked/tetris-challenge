#ifndef TIMEDGAME_H_INCLUDED
#define TIMEDGAME_H_INCLUDED


#include "ThreadSafeGame.h"
#include "Poco/Timer.h"


namespace Tetris
{

    /**
     * TimedGame adds the timed lowering of the active block.
     */
    class TimedGame
    {
    public:
        TimedGame(const ThreadSafeGame & inGame);

        void start();

        int level() const;

        // The higher the level the faster the blocks are falling.
        // Levels go from 0 to 20. If the level is set to a
        // bigger value then it is set to 20 instead.
        void setLevel(int inLevel);

        void onTimerEvent(Poco::Timer & inTimer);

    private:
        ThreadSafeGame mThreadSafeGame;
        int mLevel;
        Poco::Timer mTimer;
    };

} // namespace Tetris


#endif // TIMEDGAME_H_INCLUDED
