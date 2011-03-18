#include "Poco/Foundation.h"
#include "Tetris/Config.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/Gravity.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/Evaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Worker.h"
#include "Tetris/Threading.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Tetris/Assert.h"
#include "Poco/Environment.h"
#include "Poco/Timer.h"
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <set>


namespace Tetris {


struct ComputerPlayer::Impl : boost::noncopyable
{
public:
    typedef ComputerPlayer::Tweaker Tweaker;

    MakeTetrises a;

    Impl() :
        mComputerPlayer(0),
        mTweaker(0),
        mWorkerPool("ComputerPlayer WorkerPool", 1), //Poco::Environment::processorCount()),
        mEvaluator(new MakeTetrises()),
        mBlockMover(),
        mSearchDepth(6),
        mSearchWidth(4),
        mWorkerCount(1), //Poco::Environment::processorCount()),
        mGameDepth(0),
        mStop(false),
        mReset(false),
        mQuitFlag(false),
        mTimer(10, 10)
    {
    }

    ~Impl()
    {
        boost::mutex::scoped_lock lock(mMutex);
        if (mNodeCalculator)
        {
            mNodeCalculator->stop();
        }
        mWorkerPool.interruptAndClearQueue();
    }

    void timerEvent();

    //int calculateRemainingTimeMs(const Game & inGame) const;

    void updateComputerBlockMoveSpeed();

    void startNodeCalculator();
    void onStarted();
    void onWorking();
    void onStopped();
    void onFinished();
    void onError();

    ComputerPlayer * mComputerPlayer;
    Tweaker * mTweaker;

    WorkerPool mWorkerPool;
    boost::scoped_ptr<NodeCalculator> mNodeCalculator;
    boost::scoped_ptr<Evaluator> mEvaluator;
    boost::scoped_ptr<BlockMover> mBlockMover;
    int mSearchDepth;
    int mSearchWidth;
    int mWorkerCount;
    int mGameDepth;
    bool mStop;
    bool mReset;
    bool mQuitFlag;
    boost::mutex mMutex;
    Poco::Timer mTimer;
};


ComputerPlayer::ComputerPlayer(const TeamName & inTeamName,
                               const PlayerName & inPlayerName,
                               size_t inRowCount,
                               size_t inColumnCount) :
    Player(PlayerType_Computer, inTeamName, inPlayerName, inRowCount, inColumnCount),
    mImpl(new Impl())
{
    mImpl->mComputerPlayer = this;
    mImpl->mBlockMover.reset(new BlockMover(simpleGame()->game()));
    mImpl->mTimer.start(Poco::TimerCallback<ComputerPlayer>(*this, &ComputerPlayer::onTimerEvent));
}


ComputerPlayer::~ComputerPlayer()
{
    {
        boost::mutex::scoped_lock lock(mImpl->mMutex);
        mImpl->mQuitFlag = true;
    }
    mImpl->mTimer.stop();
    mImpl.reset();
}


void ComputerPlayer::onTimerEvent(Poco::Timer & )
{
    try
    {
        boost::mutex::scoped_lock lock(mImpl->mMutex);
        if (!mImpl->mQuitFlag)
        {
            mImpl->timerEvent();
        }
    }
    catch (const std::exception & exc)
    {
        LogError(MakeString() << "Exception caught during ComputerPlayer timerEvent. Details: " << exc.what());
    }
}


void ComputerPlayer::setTweaker(Tweaker *inTweaker)
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    mImpl->mTweaker = inTweaker;
}


int ComputerPlayer::searchDepth() const
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    return mImpl->mSearchDepth;
}


void ComputerPlayer::setSearchDepth(int inSearchDepth)
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    mImpl->mSearchDepth = inSearchDepth;
}


int ComputerPlayer::searchWidth() const
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    return mImpl->mSearchWidth;
}


void ComputerPlayer::setSearchWidth(int inSearchWidth)
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    mImpl->mSearchWidth = inSearchWidth;
}


int ComputerPlayer::currentSearchDepth() const
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    if (mImpl->mNodeCalculator)
    {
        return mImpl->mNodeCalculator->getCurrentSearchDepth();
    }
    return 0;
}


int ComputerPlayer::moveSpeed() const
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    return mImpl->mBlockMover->speed();
}


void ComputerPlayer::setMoveSpeed(int inMoveSpeed)
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    mImpl->mBlockMover->setSpeed(inMoveSpeed);
}


void ComputerPlayer::setEvaluator(std::auto_ptr<Evaluator> inEvaluator)
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    mImpl->mEvaluator.reset(inEvaluator.release());
}


int ComputerPlayer::workerCount() const
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    return mImpl->mWorkerCount;
}


void ComputerPlayer::setWorkerCount(int inWorkerCount)
{
    boost::mutex::scoped_lock lock(mImpl->mMutex);
    if (inWorkerCount == 0)
    {
        mImpl->mWorkerCount = Poco::Environment::processorCount();
    }
    else
    {
        mImpl->mWorkerCount = inWorkerCount;
    }
}


void ComputerPlayer::Impl::updateComputerBlockMoveSpeed()
{
    // Consult the Tweaker for improved settings
    if (mTweaker)
    {
        BlockMover::MoveDownBehavior moveDownBehavior = BlockMover::MoveDownBehavior_Null;
        int moveSpeed = 0;

        mEvaluator.reset(
            mTweaker->updateAIParameters(*mComputerPlayer,
                                         mSearchDepth,
                                         mSearchWidth,
                                         mWorkerCount,
                                         moveSpeed,
                                         moveDownBehavior).release());

        if (mSearchDepth < 1 || mSearchDepth > 100)
        {
            throw std::runtime_error(MakeString() << "Invalid search depth: " << mSearchDepth);
        }

        if (mSearchWidth < 1 || mSearchWidth > 100)
        {
            throw std::runtime_error(MakeString() << "Invalid search width: " << mSearchWidth);
        }

        if (mWorkerCount < 1 || mWorkerCount > 128)
        {
            throw std::runtime_error(MakeString() << "Invalid worker count: " << mWorkerCount);
        }

        if (moveSpeed != 0)
        {
            if (moveSpeed < 0)
            {
                throw std::runtime_error(MakeString() << "Invalid move speed: " << moveSpeed);
            }
            mBlockMover->setSpeed(moveSpeed);
        }

        if (moveDownBehavior != BlockMover::MoveDownBehavior_Null)
        {
            mBlockMover->setMoveDownBehavior(moveDownBehavior);
        }
    }
}


void ComputerPlayer::Impl::timerEvent()
{
    if (mReset)
    {
        mReset = false;
        mNodeCalculator.reset();
        return;
    }

    if (mStop)
    {
        mStop = false;
        mNodeCalculator->stop();
        return;
    }

    // Consult the tweaker.
    updateComputerBlockMoveSpeed();

    if (!mNodeCalculator)
    {
        startNodeCalculator();
        return;
    }

    switch (mNodeCalculator->status())
    {
        case NodeCalculator::Status_Nil:
        {
            throw std::logic_error("Status should be Status_Started or higher.");
        }
        case NodeCalculator::Status_Started:
        {
            onStarted();
            return;
        }
        case NodeCalculator::Status_Working:
        {
            onWorking();
            return;
        }
        case NodeCalculator::Status_Stopped:
        {
            onStopped();
            return;
        }
        case NodeCalculator::Status_Finished:
        {
            onFinished();
            return;
        }
        case NodeCalculator::Status_Error:
        {
            onError();
            return;
        }
        default:
        {
            throw std::invalid_argument("Invalid enum value for NodeCalculator::Status.");
        }
    }
}


void ComputerPlayer::Impl::startNodeCalculator()
{

    BlockTypes futureBlocks;
    std::auto_ptr<GameStateNode> endNode;

    // Critical section
    {
        ScopedReaderAndWriter<Game> wgame(mComputerPlayer->simpleGame()->game());
        ComputerGame & game(dynamic_cast<ComputerGame&>(*wgame.get()));


        if (game.numPrecalculatedMoves() > 8)
        {
            // We're fine for now.
            return;
        }

        // Clone the starting node
        // The end node becomes the new start node. It's like... a vantage point!
        endNode = game.endNode()->clone();
        if (endNode->gameState().isGameOver())
        {
            return;
        }

        mGameDepth = endNode->depth();
        Assert(endNode->children().empty());
        Assert(endNode->depth() >= game.currentNode()->depth());


        //
        // Create the list of future blocks
        //
        game.getFutureBlocksWithOffset(endNode->depth(), mSearchDepth, futureBlocks);
    }


    //
    // Fill list of search widths (using the same width for each level).
    //
    std::vector<int> widths;
    for (size_t idx = 0; idx != futureBlocks.size(); ++idx)
    {
        widths.push_back(mSearchWidth);
    }


    //
    // Create and start the NodeCalculator object.
    //
    Assert(mWorkerPool.getActiveWorkerCount() == 0);
    if (mWorkerCount == 0)
    {
        mWorkerCount = Poco::Environment::processorCount();
    }
    mWorkerPool.resize(mWorkerCount);
    mNodeCalculator.reset(new NodeCalculator(endNode,
                                             futureBlocks,
                                             widths,
                                             mEvaluator->clone(),
                                             mWorkerPool));

    mNodeCalculator->start();
}



void ComputerPlayer::Impl::onStarted()
{
    // Good.
}


void ComputerPlayer::Impl::onWorking()
{
    ScopedReader<Game> wgame(mComputerPlayer->simpleGame()->game());
    const ComputerGame & game(dynamic_cast<const ComputerGame&>(*wgame.get()));

    if (mGameDepth < game.endNode()->depth())
    {
        // The calculated results have become invalid. Start over.
        mReset = true;
        return;
    }

    if (game.numPrecalculatedMoves() == 0)
    {
        // Stop and use current results.
        mStop = true;
        return;
    }

    // Keep working
}


void ComputerPlayer::Impl::onStopped()
{
    // Good. Now wait until onFinished.
}


void ComputerPlayer::Impl::onFinished()
{
    mReset = true;

    NodePtr resultNode = mNodeCalculator->result();
    if (!resultNode || resultNode->gameState().isGameOver())
    {
        return;
    }

    ScopedReaderAndWriter<Game> wgame(mComputerPlayer->simpleGame()->game());
    ComputerGame & game(dynamic_cast<ComputerGame&>(*wgame.get()));

    // Check for sync problems.
    if (resultNode->depth() != game.endNode()->depth() + 1)
    {
        return;
    }

    // Ok, store the results.
    game.appendPrecalculatedNode(resultNode);
}


void ComputerPlayer::Impl::onError()
{
    mReset = true;
    LogError("ComputerPlayer: " + mNodeCalculator->errorMessage());
}


} // namespace Tetris
