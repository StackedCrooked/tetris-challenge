#ifndef BLOCKMOVER_H_INCLUDED
#define BLOCKMOVER_H_INCLUDED


#include "Tetris/Threading.h"
#include "Poco/Timer.h"


namespace Tetris
{

    class Game;

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
        BlockMover(Protected<Game> & inGame);

        enum Status
        {
            Status_Ok,
            Status_Blocked,
            Status_Error,
        };

        Status status() const;

    private:
        void onTimer(Poco::Timer & ioTimer);
        void move();   
        
        Protected<Game> & mGame;
        Poco::Timer mTimer;
        volatile Status mStatus;
    };

} // namespace Tetris


#endif // BLOCKMOVER_H_INCLUDED
