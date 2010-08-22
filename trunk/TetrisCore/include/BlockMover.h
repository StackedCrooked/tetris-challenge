#ifndef BLOCKMOVER_H_INCLUDED
#define BLOCKMOVER_H_INCLUDED


#include "ThreadSafeGame.h"
#include "Poco/Timer.h"


namespace Tetris
{

    /**
     * BlockMover
     *
     * Given a Tetris block that has a current position and a requested position. This class will
     * peridically move the current block one square closer to the target block.
     *
     * The current position is the position of the Game's active block.
     * The target position is the position of its first child element.
     *
     * This class does not create child nodes. Once no more child nodes are available it goes into
     * waiting mode until more child nodes are added.
     */
    class BlockMover
    {
    public:
        BlockMover(ThreadSafeGame & inGame);

    private:
        void onTimer(Poco::Timer & ioTimer);
        void move();   
        
        ThreadSafeGame & mGame;
        Poco::Timer mTimer;
        volatile bool mIsWaiting;
    };

} // namespace Tetris


#endif // BLOCKMOVER_H_INCLUDED
