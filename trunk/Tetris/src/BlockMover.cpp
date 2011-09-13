#include "Poco/Foundation.h"
#include "Tetris/Config.h"
#include "Tetris/BlockMover.h"
#include "Tetris/GameImpl.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Stopwatch.h"
#include "Futile/Threading.h"
#include "Futile/Assert.h"
#include "Poco/AtomicCounter.h"
#include "Poco/Timer.h"
#include <iostream>
#include <boost/bind.hpp>


namespace Tetris {


using namespace Futile;


struct BlockMover::Impl
{
    Impl(ThreadSafe<GameImpl> inGame) :
        mGame(inGame),
        mTimer(10, 10), // frequency is 100/s
        mStopwatch(),
        mNumMovesPerSecond(1),
        mMoveCount(0),
        mActualSpeed(0),
        mMoveDownBehavior(MoveDownBehavior_Move)
    {
        Poco::TimerCallback<Impl> timerCallback(*this, &Impl::onTimer);
        mTimer.start(timerCallback);
    }

    ~Impl()
    {
    }

    void onTimer(Poco::Timer & inTimer);

    int periodicInterval() const
    {
        Assert(mNumMovesPerSecond != 0);
        return static_cast<int>(0.5 + 1000.0 / static_cast<double>(mNumMovesPerSecond));
    }

    void move();

    ThreadSafe<GameImpl> mGame;
    Poco::Timer mTimer;
    Futile::Stopwatch mStopwatch;
    int mNumMovesPerSecond;
    Poco::AtomicCounter mMoveCount;
    int mActualSpeed;
    MoveDownBehavior mMoveDownBehavior;
};


BlockMover::BlockMover(ThreadSafe<GameImpl> inGame) :
    mImpl(new Impl(inGame))
{
}


BlockMover::~BlockMover()
{
    // Don't allow exceptions to escape the constructor.
    try
    {
        // Stopping the timer ensures that any callbacks finish
        // before the implementation object is destructed.
        mImpl->mTimer.stop();
        mImpl.reset();
    }
    catch (const std::exception & exc)
    {
        LogError(MakeString() << "~BlockMover throws: " << exc.what());
    }
}


void BlockMover::setSpeed(unsigned inNumMovesPerSecond)
{
    if (inNumMovesPerSecond > 1000)
    {
        inNumMovesPerSecond = 1000;
    }
    else if (inNumMovesPerSecond < 1)
    {
        inNumMovesPerSecond = 1;
    }

    mImpl->mNumMovesPerSecond = inNumMovesPerSecond;
    mImpl->mTimer.setPeriodicInterval(mImpl->periodicInterval());
}


int BlockMover::speed() const
{
    return mImpl->mNumMovesPerSecond;
}



int BlockMover::actualSpeed() const
{
    return mImpl->mActualSpeed;
}


void BlockMover::setMoveDownBehavior(MoveDownBehavior inMoveDownBehavior)
{
    mImpl->mMoveDownBehavior = inMoveDownBehavior;
}


BlockMover::MoveDownBehavior BlockMover::moveDownBehavior() const
{
    return mImpl->mMoveDownBehavior;
}


void BlockMover::Impl::onTimer(Poco::Timer &)
{
    try
    {
        move();
        ++mMoveCount;

        if (mStopwatch.elapsedMs() >= 4000)
        {
            mActualSpeed = mMoveCount / 4;
            mMoveCount = 0;
            mStopwatch.restart();
        }
    }
    catch (const std::exception & inException)
    {
        LogError(MakeString() << "Unanticipated exception thrown in Impl::move(). Details: " << inException.what());
    }
}


void BlockMover::Impl::move()
{
    Locker<GameImpl> wGame(mGame);
    ComputerGame & game = dynamic_cast<ComputerGame&>(*wGame.get());
    if (game.isPaused())
    {
        return;
    }

    const ChildNodes & children = game.currentNode()->children();
    if (children.empty())
    {
        return;
    }

    GameStateNode & firstChild = **children.begin();

    const Block & block = game.activeBlock();
    const Block & targetBlock = firstChild.gameState().originalBlock();

    Assert(block.type() == targetBlock.type());

    // Try rotation first, if it fails then skip rotation and try horizontal move
    if (block.rotation() != targetBlock.rotation())
    {
        if (game.rotate())
        {
            return;
        }
        // else: try left or right move below
    }

    if (block.column() < targetBlock.column())
    {
        if (!game.move(MoveDirection_Right))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            game.dropAndCommit();
        }
        return;
    }

    if (block.column() > targetBlock.column())
    {
        if (!game.move(MoveDirection_Left))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            game.dropAndCommit();
        }
        return;
    }

    // Horizontal position is OK.
    // Retry rotation again. If it fails here then drop the block.
    if (block.rotation() != targetBlock.rotation())
    {
        if (!game.rotate())
        {
            game.dropAndCommit();
        }
        return;
    }

    //
    // If we get arrive here then horizontal position and rotation are OK.
    // Start lowering the block.
    //
    if (mMoveDownBehavior == BlockMover::MoveDownBehavior_Move)
    {
        game.move(MoveDirection_Down);
    }
    else if (mMoveDownBehavior == BlockMover::MoveDownBehavior_Drop)
    {
        game.dropAndCommit();
    }
    else
    {
        throw std::logic_error(MakeString() << "MoveDownBehavior: invalid enum value: " << mMoveDownBehavior);
    }
}


} // namespace Tetris
