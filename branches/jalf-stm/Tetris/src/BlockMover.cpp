#include "Poco/Foundation.h"
#include "Tetris/BlockMover.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Stopwatch.h"
#include "Futile/Threading.h"
#include "Futile/Assert.h"
#include "Futile/Timer.h"
#include <iostream>
#include <boost/bind.hpp>


namespace Tetris {


using namespace Futile;


struct BlockMover::Impl
{
    struct Data
    {
        Data() :
            mStopwatch(),
            mNumMovesPerSecond(1),
            mMoveCount(0),
            mActualSpeed(0),
            mMoveDownBehavior(MoveDownBehavior_Move)
        {
            mStopwatch.start();
        }

        Futile::Stopwatch mStopwatch;
        unsigned mNumMovesPerSecond;
        unsigned mMoveCount;
        unsigned mActualSpeed;
        MoveDownBehavior mMoveDownBehavior;
    };

    Impl(SimpleGame inSimpleGame) :
        mSimpleGame(inSimpleGame),
        mTimer(10), // frequency is 100/s
        mData(new Data)
    {
    }

    ~Impl()
    {
    }

    void onTimerEvent();

    static unsigned GetTimerIntervalMs(unsigned & inNumMovesPerSecond)
    {
        return static_cast<unsigned>(0.5 + 1000.0 / static_cast<double>(inNumMovesPerSecond));
    }

    void setNumMovesPerSecond(unsigned n)
    {
        FUTILE_LOCK(Data & data, mData)
        {
            data.mNumMovesPerSecond = n;
        }
        mTimer.setInterval(GetTimerIntervalMs(n));
    }

    unsigned speed() const
    {
        return mData.lock()->mNumMovesPerSecond;
    }

    unsigned actualSpeed() const
    {
        return mData.lock()->mActualSpeed;
    }

    void setMoveDownBehavior(MoveDownBehavior inMoveDownBehavior)
    {
        FUTILE_LOCK(Data & data, mData)
        {
            data.mMoveDownBehavior = inMoveDownBehavior;
        }
    }

    MoveDownBehavior moveDownBehavior() const
    {
        return mData.lock()->mMoveDownBehavior;
    }

    void move(Data & data);

    SimpleGame mSimpleGame;
    Futile::Timer mTimer;
    ThreadSafe<Data> mData;
};


BlockMover::BlockMover(SimpleGame inSimpleGame) :
    mImpl(new Impl(inSimpleGame))
{
    mImpl->mTimer.start(boost::bind(&Impl::onTimerEvent, mImpl.get()));
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
        LogError(SS() << "~BlockMover throws: " << exc.what());
    }
}


void BlockMover::setSpeed(unsigned inNumMovesPerSecond)
{
    if (inNumMovesPerSecond > 1000)
    {
        inNumMovesPerSecond = 1000;
    }

    if (inNumMovesPerSecond < 1)
    {
        inNumMovesPerSecond = 1;
    }

    mImpl->setNumMovesPerSecond(inNumMovesPerSecond);
}


int BlockMover::speed() const
{
    return mImpl->speed();
}



int BlockMover::actualSpeed() const
{
    return mImpl->actualSpeed();
}


void BlockMover::setMoveDownBehavior(MoveDownBehavior inMoveDownBehavior)
{
    mImpl->setMoveDownBehavior(inMoveDownBehavior);
}


BlockMover::MoveDownBehavior BlockMover::moveDownBehavior() const
{
    return mImpl->moveDownBehavior();
}


void BlockMover::Impl::onTimerEvent()
{
    try
    {
        FUTILE_LOCK(Data & data, mData)
        {
            move(data);
            ++data.mMoveCount;

            if (data.mStopwatch.elapsedMs() >= 4000)
            {
                data.mActualSpeed = data.mMoveCount / 4;
                data.mMoveCount = 0;
                data.mStopwatch.restart();
            }
        }
    }
    catch (const std::exception & inException)
    {
        LogError(SS() << "Unanticipated exception thrown in Impl::move(). Details: " << inException.what());
    }
}


void BlockMover::Impl::move(Data & data)
{
}


void Move(SimpleGame & mSimpleGame, const Block & targetBlock)
{
    Block block = mSimpleGame.activeBlock();
    Assert(block.type() == targetBlock.type());

    // Try rotation first, if it fails then skip rotation and try horizontal move
    if (block.rotation() != targetBlock.rotation())
    {
        if (mSimpleGame.rotate())
        {
            return;
        }
        // else: try left or right move below
    }

    if (block.column() < targetBlock.column())
    {
        if (!mSimpleGame.move(MoveDirection_Right))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            mSimpleGame.drop();
        }
        return;
    }

    if (block.column() > targetBlock.column())
    {
        if (!mSimpleGame.move(MoveDirection_Left))
        {
            // Damn we can't move this block anymore.
            // Give up on this block.
            mSimpleGame.drop();
        }
        return;
    }

    // Horizontal position is OK.
    // Retry rotation again. If it fails here then drop the block.
    if (block.rotation() != targetBlock.rotation())
    {
        if (!mSimpleGame.rotate())
        {
            mSimpleGame.drop();
        }
        return;
    }

    mSimpleGame.move(MoveDirection_Down);
}


} // namespace Tetris
