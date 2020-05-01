#ifndef TETRIS_BLOCKMOVER_H_INCLUDED
#define TETRIS_BLOCKMOVER_H_INCLUDED


#include "Tetris/Utilities.h"
#include "Futile/Threading.h"
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>


namespace Tetris {


class GameImpl;
class BlockMover;

typedef boost::function<void(BlockMover *)> BlockMoverCallback;


/**
 * BlockMover
 *
 * A BlockMove object will peridically move the current tetris block
 * one square closer to the next precalculated block (from the AI).
 */
class BlockMover
{
public:
    BlockMover(Futile::ThreadSafe<GameImpl> inGame);

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
    BlockMover& operator=(const BlockMover&);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // BLOCKMOVER_H_INCLUDED
