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
#include "Futile/Threading.h"
#include "Futile/Assert.h"
#include "Poco/AtomicCounter.h"
#include "Poco/Stopwatch.h"
#include "Poco/Timer.h"
#include <iostream>
#include <boost/bind.hpp>


using Futile::LogError;
using Futile::MakeString;
using Futile::Locker;


namespace Tetris {


struct BlockMover::Impl
{
    Impl(ThreadSafe<GameImpl> inGame) :
        mGame(inGame),
        mTimer(),
        mStopwatch(),
        mNumMovesPerSecond(1),
        mMoveCount(0),
        mActualSpeed(0),
        mMoveDownBehavior(MoveDownBehavior_Move)
    {
    }

    ~Impl()
    {
        try
        {
            mTimer->stop();
            mTimer.reset();
        }
        catch (Poco::Exception& exc)
        {
            std::cerr << exc.what();
        }
        catch (const std::exception& exc)
        {
            std::cerr << exc.what();
        }
    }

    void onTimer(Poco::Timer& inTimer);

    int periodicInterval() const
    {
        Assert(mNumMovesPerSecond != 0);
        return static_cast<int>(0.5 + 1000.0 / static_cast<double>(mNumMovesPerSecond));
    }

    void move();

    ThreadSafe<GameImpl> mGame;
    boost::scoped_ptr<Poco::Timer> mTimer;
    Poco::Stopwatch mStopwatch;
    int mNumMovesPerSecond;
    Poco::AtomicCounter mMoveCount;
    int mActualSpeed;
    MoveDownBehavior mMoveDownBehavior;
};


BlockMover::BlockMover(ThreadSafe<GameImpl> inGame) :
    mImpl(new Impl(inGame))
{
    mImpl->mTimer.reset(new Poco::Timer(10, 10));
    Poco::TimerCallback<Impl> timerCallback(*mImpl, &Impl::onTimer);
    mImpl->mTimer->start(timerCallback);
    mImpl->mStopwatch.start();
}


BlockMover::~BlockMover()
{
    mImpl.reset();
}


void BlockMover::setSpeed(int inNumMovesPerSecond)
{
    if (inNumMovesPerSecond <= 0)
    {
        throw std::invalid_argument("Number of moves per second must be different from 0.");
    }
    if (inNumMovesPerSecond > 1000)
    {
        throw std::runtime_error(MakeString() << "Max move speed exceeded: " << inNumMovesPerSecond << "/1000");
    }

    mImpl->mNumMovesPerSecond = inNumMovesPerSecond;
    if (mImpl->mNumMovesPerSecond < 1)
    {
        mImpl->mNumMovesPerSecond = 1;
    }
    if (mImpl->mTimer)
    {
        mImpl->mTimer->setPeriodicInterval(mImpl->periodicInterval());
    }
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

        static const Poco::UInt64 cOneSecond = 1000 * 1000;
        if (mStopwatch.elapsed() >= 4 * cOneSecond)
        {
            mActualSpeed = mMoveCount / 4;
            mMoveCount = 0;
            mStopwatch.restart();
        }
    }
    catch (const std::exception& inException)
    {
        LogError(MakeString() << "Unanticipated exception thrown in Impl::move(). Details: " << inException.what());
    }
}


void BlockMover::Impl::move()
{
    Locker<GameImpl> wGame(mGame);
    ComputerGame& game = dynamic_cast<ComputerGame&>(*wGame.get());
    if (game.isPaused())
    {
        return;
    }

    const ChildNodes& children = game.currentNode()->children();
    if (children.empty())
    {
        return;
    }

    GameStateNode& firstChild = **children.begin();

    const Block& block = game.activeBlock();
    const Block& targetBlock = firstChild.gameState().originalBlock();

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
