#include "Tetris/BlockMover.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/ErrorHandling.h"
#include "Tetris/Logger.h"
#include <stdexcept>


namespace Tetris
{
    
    static int GetIntervalMs(int inNumMovesPerSecond)
    {
        Assert(inNumMovesPerSecond != 0);
        return static_cast<int>(0.5 + 1000.0 / static_cast<float>(inNumMovesPerSecond != 0 ? inNumMovesPerSecond : 1));
    }


    BlockMover::BlockMover(Protected<Game> & inGame, int inNumMovesPerSecond) :
        mGame(inGame),
        mTimer(GetIntervalMs(inNumMovesPerSecond), GetIntervalMs(inNumMovesPerSecond)),
        mStatus(Status_Ok)
    {
        Poco::TimerCallback<BlockMover> callback(*this, &BlockMover::onTimer);
        mTimer.start(callback);
    }


    BlockMover::Status BlockMover::status() const
    {
        return mStatus;
    }


    void BlockMover::setSpeed(int inNumMovesPerSecond)
    {
        Assert(inNumMovesPerSecond != 0);
        mTimer.setPeriodicInterval(GetIntervalMs(inNumMovesPerSecond));
    }


    void BlockMover::onTimer(Poco::Timer & ioTimer)
    {
        try
        {
            move();
        }
        catch (const std::exception & inException)
        {
            mStatus = Status_Error;
            LogError(inException.what());
        }
    }


    void BlockMover::move()
    {
        if (mStatus == Status_Blocked)
        {
            return;
        }

        ScopedAtom<Game> game(mGame);
        const ChildNodes & children = game->currentNode()->children();
        if (children.empty())
        {
            return;
        }

        Block & block = game->activeBlock();
        const Block & targetBlock = (*children.begin())->state().originalBlock();
        Assert(block.type() == targetBlock.type());
        if (block.rotation() != targetBlock.rotation())
        {
            if (!game->rotate())
            {
                mStatus = Status_Blocked;
            }
        }
        else if (block.column() < targetBlock.column())
        {
            if (!game->move(Direction_Right))
            {
                mStatus = Status_Blocked;
            }
        }
        else if (block.column() > targetBlock.column())
        {
            if (!game->move(Direction_Left))
            {
                mStatus = Status_Blocked;
            }
        }
        else
        {
            GameState & gameState = game->currentNode()->state();
            if (gameState.checkPositionValid(block, block.row() + 1, block.column()))
            {
                game->move(Direction_Down);
            }
            else if (!game->navigateNodeDown())
            {
                throw std::runtime_error("Failed to navigate the game state one node down.");
            }
        }
    }


} // namespace Tetris