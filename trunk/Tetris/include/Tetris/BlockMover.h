#ifndef TETRIS_BLOCKMOVER_H_INCLUDED
#define TETRIS_BLOCKMOVER_H_INCLUDED


#include "Tetris/Utilities.h"
#include <boost/function.hpp>


namespace Tetris
{

    template<class Variable> class ThreadSafe;
    class ComputerGame;
    class BlockMover;
    class BlockMoverImpl;

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
        BlockMover(const ThreadSafe<ComputerGame> & inComputerGame);

        ~BlockMover();

        void setSpeed(int inNumMovesPerSecond);

        int speed() const;

        void setInterval(int inTimeBetweenMovesInMilliseconds);

        int interval() const;

        void setCallback(const BlockMoverCallback & inBlockMoverCallback);

    private:
        BlockMover(const BlockMover &);
        BlockMover & operator=(const BlockMover&);

        BlockMoverImpl * mImpl;
    };

} // namespace Tetris


#endif // BLOCKMOVER_H_INCLUDED
