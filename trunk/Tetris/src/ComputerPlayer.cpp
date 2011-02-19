#include "Poco/Foundation.h"
#include "Tetris/Config.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/BlockMover.h"
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
    static const int cDefaultSearchDepth = 3;
    static const int cDefaultSearchWidth = 8;

    typedef ComputerPlayer::Tweaker Tweaker;

    Impl(const std::string & inName,
         const ThreadSafe<Game> & inProtectedGame,
         std::auto_ptr<Evaluator> inEvaluator) :
        mTweaker(0),
        mName(inName),
        mProtectedGame(inProtectedGame),
        mWorkerPool("ComputerPlayer WorkerPool", Poco::Environment::processorCount()),
        mEvaluator(inEvaluator.release()),
        mBlockMover(new BlockMover(mProtectedGame)),
        mSearchDepth(cDefaultSearchDepth),
        mSearchWidth(cDefaultSearchWidth),
        mWorkerCount(Poco::Environment::processorCount()),
        mGameDepth(0),
        mStop(false),
        mReset(false),
        mQuitFlag(false),
        mTimer(15, 15)
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

    int calculateRemainingTimeMs(const Game & inGame) const;

    void startNodeCalculator();
    void onStarted();
    void onWorking();
    void onStopped();
    void onFinished();
    void onError();

    Tweaker * mTweaker;
    std::string mName;
    ThreadSafe<Game> mProtectedGame;
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


ComputerPlayer::ComputerPlayer(const std::string & inName,
                               const ThreadSafe<Game> & inProtectedGame,
                               std::auto_ptr<Evaluator> inEvaluator) :
    mImpl(new Impl(inName, inProtectedGame, inEvaluator))
{
    mImpl->mTimer.start(Poco::TimerCallback<ComputerPlayer>(*this, &ComputerPlayer::onTimerEvent));
}


ComputerPlayer::~ComputerPlayer()
{
    {
        boost::mutex::scoped_lock lock(mImpl->mMutex);
        mImpl->mQuitFlag = true;
    }
    mImpl->mTimer.stop();
    delete mImpl;
}


const std::string & ComputerPlayer::name() const
{
    return mImpl->mName;
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


int ComputerPlayer::Impl::calculateRemainingTimeMs(const Game & inGame) const
{
    // Already locked.

    const ComputerGame & game(dynamic_cast<const ComputerGame&>(inGame));

    int firstOccupiedRow = game.currentNode()->gameState().firstOccupiedRow();
    int currentBlockRow = game.activeBlock().row();
    int numBlockRows = std::max<int>(game.activeBlock().grid().rowCount(), game.activeBlock().grid().columnCount());
    int numRemainingRows = firstOccupiedRow - (currentBlockRow + numBlockRows);
    if (numRemainingRows <= 2)
    {
        return 0;
    }

    double numRowsPerSecond = Gravity::CalculateSpeed(game.level());
    double remainingTime = 1000 * static_cast<double>(numRemainingRows) / numRowsPerSecond;
    int maxRequiredMoves = game.activeBlock().numRotations() + (game.columnCount()/2);
    int moveSpeed = mBlockMover->speed();
    double timeRequiredForMove = 1000.0 * static_cast<double>(maxRequiredMoves) / static_cast<double>(moveSpeed);
    return static_cast<int>(0.5 + remainingTime - timeRequiredForMove);
}


void ComputerPlayer::Impl::timerEvent()
{
    // Already locked.

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
        ScopedReader<Game> wgame(mProtectedGame);
        const ComputerGame & game(dynamic_cast<const ComputerGame&>(*wgame.get()));

        // Consult the Tweaker for improved settings
        if (mTweaker)
        {
            mEvaluator.reset(
                mTweaker->updateAIParameters(game.gameState(),
                                             mSearchDepth,
                                             mSearchWidth,
                                             mWorkerCount).release());
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
    ScopedReader<Game> wgame(mProtectedGame);
    const ComputerGame & game(dynamic_cast<const ComputerGame&>(*wgame.get()));

    if (mGameDepth < game.endNode()->depth())
    {
        // The calculated results have become invalid. Start over.
        mReset = true;
        return;
    }

    if (game.numPrecalculatedMoves() == 0 && calculateRemainingTimeMs(game) < 1000)
    {
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

    ScopedReaderAndWriter<Game> wgame(mProtectedGame);
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
    LogError(mName + " " + mNodeCalculator->errorMessage());
}


} // namespace Tetris
