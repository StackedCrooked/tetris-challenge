#ifndef TIMEDGAME_H_INCLUDED
#define TIMEDGAME_H_INCLUDED


#include "Tetris/Threading.h"
#include "Poco/Timer.h"


namespace Tetris
{
    class Game;

    /**
     * TimedGame adds the timed lowering of the active block.
     */
    class TimedGame
    {
    public:
        TimedGame(const Protected<Game> & inGame);

        void start();

        int level() const;

        // The higher the level the faster the blocks are falling.
        // Levels go from 0 to 20. If the level is set to a
        // bigger value then it is set to 20 instead.
        void setLevel(int inLevel);        

    private:
        void onTimerEvent(Poco::Timer & inTimer);

        Protected<Game> mThreadSafeGame;
        int mLevel;
        Poco::Timer mTimer;
    };

} // namespace Tetris


#endif // TIMEDGAME_H_INCLUDED
