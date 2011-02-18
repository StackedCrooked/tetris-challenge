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

    int calculateRemainingTimeMs(const Game & inGame) const;

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

    if (mNodeCalculator)
    {
        // Apparently an error has occured, which has been logged by the NodeCalculator object.
        // All we can do at this point is reset the pointer so that a new object will be created
        // at the next timer event.
        if (mNodeCalculator->status() == NodeCalculator::Status_Error)
        {
            mNodeCalculator.reset();
            return;
        }

        // Check if the computer player has finished.
        if (mNodeCalculator->status() != NodeCalculator::Status_Finished)
        {
            int numPrecalculatedMoves = -1;
            int remainingTime = -1;
            {
                ScopedReader<Game> wgame(mProtectedGame);
                const ComputerGame & game(dynamic_cast<const ComputerGame&>(*wgame.get()));

                //
                // Check if the game state has not been changed while calculating the next move
                //
                if (mGameDepth < game.lastPrecalculatedNode()->depth())
                {
                    LogInfo("AAARGGHH All our work is for naught");

                    // Game state has change, so all calculations up until now become invalid.
                    // Reset the node calculator and so that it will be be restarted in the next iteration.
                    mNodeCalculator.reset();
                    return;
                }

                numPrecalculatedMoves = game.numPrecalculatedMoves();
                if (numPrecalculatedMoves == 0)
                {
                    remainingTime = calculateRemainingTimeMs(game);
                }
            }

            if (numPrecalculatedMoves == 0)
            {
                // Check if there is the danger of crashing the current block.

                if (remainingTime <= 1000)
                {
                    mNodeCalculator->stop();
                }
            }
            // else: keep working.
        }
        else
        {
            if (NodePtr resultNode = mNodeCalculator->result())
            {
                if (!resultNode->gameState().isGameOver())
                {
                    ScopedReaderAndWriter<Game> wgame(mProtectedGame);
                    ComputerGame & game(dynamic_cast<ComputerGame&>(*wgame.get()));

                    // The created node should follow the last precalculated one.
                    if (resultNode->depth() == game.lastPrecalculatedNode()->depth() + 1)
                    {
                        game.appendPrecalculatedNode(resultNode);

                    }
                    else
                    {
                        LogWarning("Computer is TOO SLOW!!");
                    }
                }
            }
            else
            {
                LogError("NodeCalculator did not create any results.");
            }

            // Once the computer has finished it's job we destroy the object.
            mNodeCalculator.reset();
        }
    }
    else
    {
        ScopedReader<Game> wgame(mProtectedGame);
        const ComputerGame & game(dynamic_cast<const ComputerGame&>(*wgame.get()));
        if (!game.lastPrecalculatedNode()->gameState().isGameOver())
        {
            mGameDepth = game.lastPrecalculatedNode()->depth();
            int numPrecalculated = mGameDepth - game.currentNode()->depth();
            if (numPrecalculated < 8)
            {
                Assert(!mNodeCalculator);

                //
                // Clone the starting node
                //
                std::auto_ptr<GameStateNode> endNode = game.lastPrecalculatedNode()->clone();
                Assert(endNode->children().empty());
                Assert(endNode->depth() >= game.currentNode()->depth());


                //
                // Create the list of future blocks
                //
                BlockTypes futureBlocks;
                game.getFutureBlocksWithOffset(endNode->depth(), mSearchDepth, futureBlocks);


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

                if (mTweaker)
                {
                    mEvaluator.reset(
                        mTweaker->updateAIParameters(game.gameState(),
                                                     mSearchDepth,
                                                     mSearchWidth,
                                                     mWorkerCount).release());
                }
                mNodeCalculator.reset(new NodeCalculator(endNode,
                                                         futureBlocks,
                                                         widths,
                                                         mEvaluator->clone(),
                                                         mWorkerPool));

                mNodeCalculator->start();
            }
            // else: we have plenty of precalculated nodes. Do nothing for know.
        }
    }
}


} // namespace Tetris
