#include "BlockMover.h"
#include "Logger.h"


namespace Tetris
{

    BlockMover::BlockMover(ThreadSafeGame & inGame) :
        mGame(inGame),
        mTimer(0, 50)
    {
        Poco::TimerCallback<BlockMover> callback(*this, &BlockMover::onTimer);
        mTimer.start(callback);
    }


    void BlockMover::onTimer(Poco::Timer & ioTimer)
    {
        try
        {
            move();
        }
        catch (const std::exception & inException)
        {
            LogError(inException.what());

            // Critical section
            {
                WritableGame wg(mGame);
                Game & game = *wg.get();
                
                // Try to cheat our way out of it
                LogWarning("I can't move the block to where I planned to so I'm cheating now.");
                if (!game.navigateNodeDown())
                {
                    mTimer.stop();
                    LogError("Application state has become corrupted for an unknown reason.");
                }
            }
        }
    }


    void BlockMover::move()
    {
        // Critical section
        while (true)
        {
            WritableGame game(mGame);
            const ChildNodes & children = game->currentNode()->children();
            if (children.empty())
            {
                // Do nothing. Perhaps we'll have more luck during the next timer event.
                return;
            }

            Block & block = game->activeBlock();
            const Block & targetBlock = (*children.begin())->state().originalBlock();
            assert(block.type() == targetBlock.type());
            if (block.rotation() != targetBlock.rotation())
            {
                if (!game->rotate())
                {
                    throw std::runtime_error(MakeString() << "Rotation failed.");
                }
            }
            else if (block.column() < targetBlock.column())
            {
                if (!game->move(Direction_Right))
                {
                    throw std::runtime_error(MakeString() << "Move to right failed. Current column: " << block.column()
                                                          << ", target column: " << targetBlock.column() << ".");
                }
            }
            else if (block.column() > targetBlock.column())
            {
                if (!game->move(Direction_Left))
                {
                    throw std::runtime_error(MakeString() << "Move to left failed. "
                                                          << "Current column: " << block.column() << ", "
                                                          << "target column: " << targetBlock.column() << ".");
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
            break;
        }
    }


} // namespace Tetris