#ifndef TETRIS_BLOCKMOVER_H_INCLUDED
#define TETRIS_BLOCKMOVER_H_INCLUDED


#include "Tetris/Utilities.h"
#include <boost/function.hpp>


namespace Tetris {


template<class Variable> class ThreadSafe;
class Game;
class BlockMover;

typedef boost::function<void(BlockMover *)> BlockMoverCallback;


/**
 * BlockMover
 *
 * A BlockMove object will peridically move the current tetris block
 * one square closer to the next precalculated block (from the AI).
 *
 * If no precalculated blocks are available then it will enter a waiting state.
 */
class BlockMover
{
public:
    BlockMover(ThreadSafe<Game> inGame);

    ~BlockMover();

    void setSpeed(int inNumMovesPerSecond);

    int speed() const;

    int actualSpeed() const;

    enum MoveDownBehavior
    {
        MoveDownBehavior_Null,
        MoveDownBehavior_Move,
        MoveDownBehavior_Drop
    };

    void setMoveDownBehavior(MoveDownBehavior inMoveDownBehavior);

    MoveDownBehavior moveDownBehavior() const;

private:
    BlockMover(const BlockMover &);
    BlockMover & operator=(const BlockMover&);

    struct Impl;
    Impl * mImpl;
};


} // namespace Tetris


#endif // BLOCKMOVER_H_INCLUDED
