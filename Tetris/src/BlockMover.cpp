#include "Poco/Foundation.h"
#include "Tetris/Config.h"
#include "Tetris/BlockMover.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Tetris/Threading.h"
#include "Tetris/Assert.h"
#include "Poco/Stopwatch.h"
#include "Poco/Timer.h"
#include <boost/bind.hpp>


namespace Tetris {


struct BlockMover::Impl
{
    Impl(ThreadSafe<Game> inGame) :
        mGame(inGame),
        mTimer(),
        mStopwatch(),
        mNumMovesPerSecond(1)
    {
    }

    ~Impl()
    {
        mTimer->stop();
        mTimer.reset();
    }

    void onTimer(Poco::Timer & inTimer);

    int periodicInterval() const
    {
        Assert(mNumMovesPerSecond != 0);
        return static_cast<int>(0.5 + 1000.0 / static_cast<double>(mNumMovesPerSecond));
    }


    void move();

    ThreadSafe<Game> mGame;
    boost::scoped_ptr<Poco::Timer> mTimer;
    Poco::Stopwatch mStopwatch;
    int mNumMovesPerSecond;
};


BlockMover::BlockMover(ThreadSafe<Game> inGame) :
    mImpl(new Impl(inGame))
{
    mImpl->mTimer.reset(new Poco::Timer(10, 10));
    Poco::TimerCallback<Impl> timerCallback(*mImpl, &Impl::onTimer);
    mImpl->mTimer->start(timerCallback);
    mImpl->mStopwatch.start();
}


BlockMover::~BlockMover()
{
    delete mImpl;
    mImpl = 0;
}


void BlockMover::Impl::onTimer(Poco::Timer &)
{
    try
    {
        move();
    }
    catch (const std::exception & inException)
    {
        LogError(MakeString() << "Unanticipated exception thrown in Impl::move(). Details: " << inException.what());
    }
}


int BlockMover::speed() const
{
    return mImpl->mNumMovesPerSecond;
}


void BlockMover::setSpeed(int inNumMovesPerSecond)
{
    if (inNumMovesPerSecond == 0)
    {
        throw std::invalid_argument("Number of moves per second must be different from 0.");
    }

    mImpl->mNumMovesPerSecond = inNumMovesPerSecond;
    if (mImpl->mTimer)
    {
        mImpl->mTimer->setPeriodicInterval(mImpl->periodicInterval());
    }
}


void BlockMover::Impl::move()
{
    ScopedReaderAndWriter<Game> wGame(mGame);
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
        if (!game.move(MoveDirection_Right))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            game.drop();
        }
    }
    else if (block.column() > targetBlock.column())
    {
        if (!game.move(MoveDirection_Left))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            game.drop();
        }
    }
    else
    {
        game.move(MoveDirection_Down);
    }
}


} // namespace Tetris
