#include "Tetris/BlockMover.h"
#include "Tetris/Assert.h"
#include "Tetris/Game.h"
#include "Tetris/GameState.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Logger.h"
#include "Tetris/MakeString.h"
#include "Poco/Timer.h"
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
        mNumMovesPerSecond(inNumMovesPerSecond),
        mMoveDownBehavior(MoveDownBehavior_Move)
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


    void BlockMover::setMoveDownBehavior(MoveDownBehavior inMoveDown)
    {
        mMoveDownBehavior = inMoveDown;
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
        ScopedAtom<Game> wgame(mGame, 100);
        Game & game = *wgame.get();

        const ChildNodes & children = game.currentNode()->children();
        if (children.empty())
        {
            return;
        }

        GameStateNode & firstChild = **children.begin();

        Block & block = game.activeBlock();
        const Block & targetBlock = firstChild.state().originalBlock();
        Assert(block.type() == targetBlock.type());
        if (block.rotation() != targetBlock.rotation())
        {
            if (!game.rotate())
            {
                // Damn we can't rotate.
                // Give up on this block.
                game.drop();
            }
        }
        else if (block.column() < targetBlock.column())
        {
            if (!game.move(Direction_Right))
            {
                // Damn we can't move this block anymore.
                // Give up on this block.
                game.drop();
            }
        }
        else if (block.column() > targetBlock.column())
        {
            if (!game.move(Direction_Left))
            {
                // Damn we can't move this block anymore.
                // Give up on this block.
                game.drop();
            }
        }
        else
        {
            switch (mMoveDownBehavior)
            {
                case MoveDownBehavior_Move:
                {
                    game.move(Direction_Down);
                    break;
                }
                case MoveDownBehavior_Drop:
                {
                    game.drop();
                    break;
                }
                case MoveDownBehavior_DontMove:
                {
                    // Do nothing.
                    break;
                }
                default:
                {
                    throw std::logic_error("Invalid enum value.");
                }
            }
        }
    }


} // namespace Tetris
