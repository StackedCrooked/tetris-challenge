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
        if (inNumMovesPerSecond <= 0)
        {
            return 1;
        }
        return static_cast<int>(0.5 + 1000.0 / static_cast<float>(inNumMovesPerSecond != 0 ? inNumMovesPerSecond : 1));
    }


    BlockMover::BlockMover(Protected<Game> & inGame, int inNumMovesPerSecond) :
        mGame(inGame),
        mTimer(),
        mNumMovesPerSecond(inNumMovesPerSecond)
    {
        mTimer.reset(new Poco::Timer(GetIntervalMs(inNumMovesPerSecond), GetIntervalMs(inNumMovesPerSecond)));
        Poco::TimerCallback<BlockMover> callback(*this, &BlockMover::onTimer);
        mTimer->start(callback);
    }


    BlockMover::~BlockMover()
    {
        mTimer->stop();
        mTimer.reset();
    }


    int BlockMover::speed() const
    {
        return mNumMovesPerSecond;
    }


    void BlockMover::setSpeed(int inNumMovesPerSecond)
    {
        if (inNumMovesPerSecond > 0)
        {
            mNumMovesPerSecond = inNumMovesPerSecond;
            mTimer->setPeriodicInterval(GetIntervalMs(inNumMovesPerSecond));
        }
    }


    void BlockMover::onTimer(Poco::Timer & ioTimer)
    {
        try
        {
            move();
        }
        catch (const LockTimeout & inException)
        {
            LogWarning(MakeString() << "BlockMover failed to lock the Game due to timeout. Details: " << inException.what());
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Unanticipated exception thrown in BlockMover::move(). Details: " << inException.what());
        }
    }


    void BlockMover::move()
    {

        ScopedAtom<Game> game(mGame, 100);
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
                // Damn we can't rotate.
                // Give up on this block.
                game->drop();
            }
        }
        else if (block.column() < targetBlock.column())
        {
            if (!game->move(Direction_Right))
            {
                // Damn we can't move this block anymore.
                // Give up on this block.
                game->drop();
            }
        }
        else if (block.column() > targetBlock.column())
        {
            if (!game->move(Direction_Left))
            {
                // Damn we can't move this block anymore.
                // Give up on this block.
                game->drop();
            }
        }
        else
        {
            GameState & gameState = game->currentNode()->state();
            if (gameState.checkPositionValid(block, block.row() + 1, block.column()))
            {
                game->move(Direction_Down);
            }
            if (!game->navigateNodeDown())
            {
                throw std::runtime_error("Unable to navigate one node down in the the gamestate tree.");
            }
        }
    }


} // namespace Tetris
