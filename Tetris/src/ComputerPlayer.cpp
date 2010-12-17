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
#include <set>
#include <boost/bind.hpp>


namespace Tetris
{

    class ComputerPlayerImpl
    {
    public:
        ComputerPlayerImpl(const Protected<Game> & inProtectedGame,
                           std::auto_ptr<Evaluator> inEvaluator,
                           int inSearchDepth,
                           int inSearchWidth,
                           int inWorkerCount);

        ComputerPlayerImpl(const Protected<Game> & inProtectedGame,
                           const ComputerPlayer::GetEvaluatorCallback & inGetEvaluator,
                           int inSearchDepth,
                           int inSearchWidth,
                           int inWorkerCount);

        ~ComputerPlayerImpl()
        {
            if (mNodeCalculator)
            {
                mNodeCalculator->stop();
            }
        }

        inline int searchDepth() const
        { return mSearchDepth; }

        inline void setSearchDepth(int inSearchDepth)
        { mSearchDepth = inSearchDepth; }

        inline int searchWidth() const
        { return mSearchWidth; }

        inline void setSearchWidth(int inSearchWidth)
        { mSearchWidth = inSearchWidth; }

        inline int moveSpeed() const
        { return mBlockMover->speed(); }

        inline void setMoveSpeed(int inMoveSpeed)
        { mBlockMover->setSpeed(inMoveSpeed); }

        int currentSearchDepth() const;

        void setEvaluator(std::auto_ptr<Evaluator> inEvaluator)
        { mEvaluator.reset(inEvaluator.release()); }

        const Evaluator & evaluator() const;

        void setDelayedEvaluator(const ComputerPlayer::GetEvaluatorCallback & inGetEvaluator)
        { mGetEvaluator = inGetEvaluator; }

        void removeEvaluator()
        { mGetEvaluator = ComputerPlayer::GetEvaluatorCallback(); }

        int workerCount() const
        { return mWorkerCount; }

        void setWorkerCount(int inWorkerCount);

    private:
        ComputerPlayerImpl(const ComputerPlayerImpl&);
        ComputerPlayerImpl& operator=(const ComputerPlayerImpl&);

        void onTimerEvent(Poco::Timer & inTimer);

        void timerEvent();

        int calculateRemainingTimeMs(const Game & game) const;
        Protected<Game> mProtectedGame;
        WorkerPool mWorkerPool;
        boost::scoped_ptr<NodeCalculator> mNodeCalculator;
        boost::scoped_ptr<Evaluator> mEvaluator;
        ComputerPlayer::GetEvaluatorCallback mGetEvaluator;
        boost::scoped_ptr<BlockMover> mBlockMover;
        Poco::Timer mTimer;
        int mSearchDepth;
        int mSearchWidth;
        int mMoveSpeed;
        int mWorkerCount;
    };


    // Use roughly 75% of all available CPUs.
    static unsigned int GetWorkerCount()
    {
        int cpuCount = Poco::Environment::processorCount();
        Assert(cpuCount >= 1);
        if (cpuCount > 1)
        {
            cpuCount = static_cast<int>(0.5 + (0.75 * static_cast<double>(cpuCount)));
        }
        return cpuCount;
    }


    ComputerPlayerImpl::ComputerPlayerImpl(const Protected<Game> & inProtectedGame,
                                           std::auto_ptr<Evaluator> inEvaluator,
                                           int inSearchDepth,
                                           int inSearchWidth,
                                           int inWorkerCount) :
        mProtectedGame(inProtectedGame),
        mWorkerPool("ComputerPlayer WorkerPool", inWorkerCount > 0 ? inWorkerCount : GetWorkerCount()),
        mEvaluator(inEvaluator.release()),
        mGetEvaluator(),
        mBlockMover(new BlockMover(mProtectedGame, 20)),
        mTimer(10, 10),
        mSearchDepth(inSearchDepth),
        mSearchWidth(inSearchWidth),
        mMoveSpeed(20),
        mWorkerCount(0)
    {
        LogInfo(MakeString() << "ComputerPlayer started with " << mWorkerPool.size() << " worker threads.");
        mTimer.start(Poco::TimerCallback<ComputerPlayerImpl>(*this, &ComputerPlayerImpl::onTimerEvent));
    }


    ComputerPlayerImpl::ComputerPlayerImpl(const Protected<Game> & inProtectedGame,
                                           const ComputerPlayer::GetEvaluatorCallback & inGetEvaluator,
                                           int inSearchDepth,
                                           int inSearchWidth,
                                           int inWorkerCount) :
        mProtectedGame(inProtectedGame),
        mWorkerPool("ComputerPlayer WorkerPool", inWorkerCount > 0 ? inWorkerCount : GetWorkerCount()),
        mEvaluator(),
        mGetEvaluator(inGetEvaluator),
        mBlockMover(new BlockMover(mProtectedGame, 20)),
        mTimer(10, 10),
        mSearchDepth(inSearchDepth),
        mSearchWidth(inSearchWidth),
        mMoveSpeed(20),
        mWorkerCount(0)
    {
        LogInfo(MakeString() << "ComputerPlayer started with " << mWorkerPool.size() << " worker threads.");
        mTimer.start(Poco::TimerCallback<ComputerPlayerImpl>(*this, &ComputerPlayerImpl::onTimerEvent));

        ScopedReader<Game> game(mProtectedGame);
        mEvaluator.reset(mGetEvaluator(game->currentNode()->state()).release());
    }
        
        
    const Evaluator & ComputerPlayerImpl::evaluator() const
    {
        Assert(mEvaluator);
        return *mEvaluator;
    }


    int ComputerPlayerImpl::calculateRemainingTimeMs(const Game & game) const
    {
        int firstOccupiedRow = game.currentNode()->state().stats().firstOccupiedRow();
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


    int ComputerPlayerImpl::currentSearchDepth() const
    {
        if (mNodeCalculator)
        {
            return mNodeCalculator->getCurrentSearchDepth();
        }
        return 0;
    }


    void ComputerPlayerImpl::setWorkerCount(int inWorkerCount)
    {
        if (inWorkerCount == 0)
        {
            mWorkerCount = GetWorkerCount();
        }
        else
        {
            mWorkerCount = inWorkerCount;
        }
    }

    
    void ComputerPlayerImpl::onTimerEvent(Poco::Timer & inTimer)
    {
        try
        {
            timerEvent();
        }
        catch (const std::exception & exc)
        {
            LogError(MakeString() << "ComputerPlayerImpl::onTimerEvent: " << exc.what());
        }
    }

    
    void ComputerPlayerImpl::timerEvent()
    {
        if (mNodeCalculator)
        {
            // Check if the computer player has finished.
            if (mNodeCalculator->status() != NodeCalculator::Status_Finished)
            {    
                int numPrecalculatedMoves = -1;
                int remainingTime = -1;
                {
                    ScopedReader<Game> wgame(mProtectedGame);
                    const Game & game(*wgame.get());
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
                    if (!resultNode->state().isGameOver())
                    {
                        ScopedReaderAndWriter<Game> wgame(mProtectedGame);
                        Game & game(*wgame.get());

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
            const Game & game(*wgame.get());
            if (!game.lastPrecalculatedNode()->state().isGameOver())
            {
                int numPrecalculated = game.lastPrecalculatedNode()->depth() - game.currentNode()->depth();
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
                    // Fill the std::vector<int> vector (use the same width for each level).
                    //
                    std::vector<int> widths;
                    for (size_t idx = 0; idx != futureBlocks.size(); ++idx)
                    {
                        widths.push_back(mSearchWidth);
                    }


                    //
                    // Create and start the NodeCalculator.
                    //
                    Assert(mWorkerPool.getActiveWorkerCount() == 0);
                    if (mWorkerCount == 0)
                    {
                        mWorkerCount = GetWorkerCount();
                    }
                    mWorkerPool.resize(mWorkerCount);

                    if (mGetEvaluator)
                    {
                        mEvaluator.reset(mGetEvaluator(game.currentNode()->state()).release());
                    }
                    mNodeCalculator.reset(new NodeCalculator(endNode, futureBlocks, widths, mEvaluator->clone(), mWorkerPool));
                    mNodeCalculator->start();
                }
                // else: we have plenty of precalculated nodes. Do nothing for know.
            }
        }
    }


    ComputerPlayer::ComputerPlayer(const Protected<Game> & inProtectedGame,
                                   std::auto_ptr<Evaluator> inEvaluator,
                                   int inSearchDepth,
                                   int inSearchWidth,
                                   int inWorkerCount) :
        mImpl(new ComputerPlayerImpl(inProtectedGame, inEvaluator, inSearchDepth, inSearchWidth, inWorkerCount))
    {
    }


    ComputerPlayer::ComputerPlayer(const Protected<Game> & inProtectedGame,
                                   const GetEvaluatorCallback & inGetEvaluator,
                                   int inSearchDepth,
                                   int inSearchWidth,
                                   int inWorkerCount) :
        mImpl(new ComputerPlayerImpl(inProtectedGame, inGetEvaluator, inSearchDepth, inSearchWidth, inWorkerCount))
    {
    }


    ComputerPlayer::~ComputerPlayer()
    {
        delete mImpl;
        mImpl = 0;
    }


    int ComputerPlayer::searchDepth() const
    {
        return mImpl->searchDepth();
    }


    void ComputerPlayer::setSearchDepth(int inSearchDepth)
    {
        mImpl->setSearchDepth(inSearchDepth);
    }


    int ComputerPlayer::currentSearchDepth() const
    {
        return mImpl->currentSearchDepth();
    }

    
    int ComputerPlayer::searchWidth() const
    {
        return mImpl->searchWidth();
    }


    void ComputerPlayer::setSearchWidth(int inSearchWidth)
    {
        mImpl->setSearchWidth(inSearchWidth);
    }

    
    int ComputerPlayer::moveSpeed() const
    {
        return mImpl->moveSpeed();
    }


    void ComputerPlayer::setMoveSpeed(int inMoveSpeed)
    {
        mImpl->setMoveSpeed(inMoveSpeed);
    }
    
    
    void ComputerPlayer::setEvaluator(std::auto_ptr<Evaluator> inEvaluator)
    {
        mImpl->setEvaluator(inEvaluator);
    }
    
    
    const Evaluator & ComputerPlayer::evaluator() const
    {
        return mImpl->evaluator();
    }


    void ComputerPlayer::setDelayedEvaluator(const GetEvaluatorCallback & inGetEvaluatorCallback)
    {
        mImpl->setDelayedEvaluator(inGetEvaluatorCallback);
    }


    void ComputerPlayer::removeDelayedEvaluator()
    {
        mImpl->removeEvaluator();
    }
    
    
    int ComputerPlayer::workerCount() const
    {
        return mImpl->workerCount();
    }

    
    void ComputerPlayer::setWorkerCount(int inWorkerCount)
    {
        mImpl->setWorkerCount(inWorkerCount);
    }

} // namespace Tetris