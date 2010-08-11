#ifndef GAMETIMER_H_INCLUDED
#define GAMETIMER_H_INCLUDED


#include "Game.h"
#include "Poco/Timer.h"


namespace Tetris
{

    /**
     * GameTimer adds the timed lowering of the active block.
     */
    class GameTimer
    {
    public:
        GameTimer(Game * inGame);

        void start();

        int level() const;

        // The higher the level the faster the blocks are falling.
        // Levels go from 0 to 20. If the level is set to a
        // bigger value then it is set to 20 instead.
        void setLevel(int inLevel);

        void onTimerEvent(Poco::Timer & inTimer);

    private:
        Game * mGame;
        int mLevel;
        Poco::Timer mTimer;
    };

} // namespace Tetris


#endif // GAMETIMER_H_INCLUDED
