#ifndef BLOCKMOVER_H_INCLUDED
#define BLOCKMOVER_H_INCLUDED


#include "Tetris/Threading.h"
#include "Poco/Timer.h"
#include <boost/noncopyable.hpp>


namespace Tetris
{

    class Game;

    /**
     * BlockMover
     *
     * A BlockMove object will peridically move the current tetris block
     * one square closer to the next precalculated block (from the AI).
     *
     * If no precalculated blocks are available then it will enter a waiting state.
     */
    class BlockMover : boost::noncopyable
    {
    public:
        BlockMover(Protected<Game> & inGame, int inNumMovesPerSecond);

        ~BlockMover();

        // Set the move speed.
        void setSpeed(int inNumMovesPerSecond);

        int speed() const;

        enum MoveDownBehavior
        {
            MoveDownBehavior_Move,      // also move the block down at the same speed at left/right/rotate
            MoveDownBehavior_DontMove,  // don't move the block down, let the gravity do the job (extra time allows for more AI precalculation)
            MoveDownBehavior_Drop       // drop the block
        };

        // Define if and how the BlockMover should move the current block down.
        void setMoveDownBehavior(MoveDownBehavior inMoveDown);

    private:
        void onTimer(Poco::Timer & ioTimer);
        void move();

        Protected<Game> & mGame;
        boost::scoped_ptr<Poco::Timer> mTimer;
        int mNumMovesPerSecond;
        MoveDownBehavior mMoveDownBehavior;
    };

} // namespace Tetris


#endif // BLOCKMOVER_H_INCLUDED
