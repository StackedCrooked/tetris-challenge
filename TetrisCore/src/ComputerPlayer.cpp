#include "Tetris/Config.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/NodeCalculator.h"
#include "Tetris/AISupport.h"
#include "Tetris/BlockMover.h"
#include "Tetris/Gravity.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateComparisonFunctor.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameState.h"
#include "Tetris/Block.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Worker.h"
#include "Tetris/Threading.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Tetris/Assert.h"
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
                           int inSearchWidth);

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

        const Evaluator & evaluator() const
        { return *mEvaluator; }

    private:
        void onTimerEvent(Poco::Timer & inTimer);

        void timerEvent();

        int calculateRemainingTimeMs(Game & game) const;
        Protected<Game> mProtectedGame;
        WorkerPool mWorkerPool;
        boost::scoped_ptr<NodeCalculator> mNodeCalculator;
        boost::scoped_ptr<Evaluator> mEvaluator;
        boost::scoped_ptr<BlockMover> mBlockMover;
        Poco::Timer mTimer;
        int mSearchDepth;
        int mSearchWidth;
        int mMoveSpeed;
    };


    ComputerPlayerImpl::ComputerPlayerImpl(const Protected<Game> & inProtectedGame,
                                           std::auto_ptr<Evaluator> inEvaluator,
                                           int inSearchDepth,
                                           int inSearchWidth) :
        mProtectedGame(inProtectedGame),
        mWorkerPool("ComputerPlayer WorkerPool", 2),
        mEvaluator(inEvaluator.release()),
        mBlockMover(new BlockMover(mProtectedGame, 20)),
        mTimer(10, 10),
        mSearchDepth(inSearchDepth),
        mSearchWidth(inSearchWidth),
        mMoveSpeed(20)
    {
        mTimer.start(Poco::TimerCallback<ComputerPlayerImpl>(*this, &ComputerPlayerImpl::onTimerEvent));
    }


    int ComputerPlayerImpl::calculateRemainingTimeMs(Game & game) const
    {
        float numRemainingRows = static_cast<float>(game.currentNode()->state().stats().firstOccupiedRow() - (game.activeBlock().row() + 4));
        float numRowsPerSecond = Gravity::CalculateSpeed(game.level());
        float remainingTime = 1000 * numRemainingRows / numRowsPerSecond;
        float timeRequiredForMove = static_cast<float>(game.activeBlock().numRotations() + game.numColumns()) / static_cast<float>(mBlockMover->speed());
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
        boost::scoped_ptr<Game> clonedGamePtr;
        {
            ScopedAtom<Game> wgame(mProtectedGame);
            clonedGamePtr.reset(wgame->clone().release());
        }
        Game & clonedGame(*clonedGamePtr);
        if (mNodeCalculator)
        {
            // Check if the computer player has finished.
            if (mNodeCalculator->status() != NodeCalculator::Status_Finished)
            {
                // Check if there is the danger of crashing the current block.
                if (clonedGame.numPrecalculatedMoves() == 0 && calculateRemainingTimeMs(clonedGame) < 1500)
                {
                    mNodeCalculator->stop();
                }
                // else: keep working.
            }
            else
            {
                NodePtr resultNode = mNodeCalculator->result();
                if (!resultNode->state().isGameOver())
                {
                    // The created node should follow the last precalculated one.
                    if (resultNode->depth() == clonedGame.lastPrecalculatedNode()->depth() + 1)
                    {
                        const int resultNodeDepth = resultNode->depth();

                        ScopedAtom<Game> wgame(mProtectedGame);
                        clonedGamePtr.reset(wgame->clone().release());
                        Game & game(*wgame.get());

                        Assert(game.lastPrecalculatedNode()->depth() + 1 == resultNodeDepth);
                        game.lastPrecalculatedNode()->addChild(resultNode);

                    }
                    else
                    {
                        LogWarning("Computer is TOO SLOW!!");
                    }
                }
                else
                {
                    // Game over. Too bad.
                    mNodeCalculator.reset();
                    return;
                }                

                // Once the computer has finished it's job we destroy the object.
                mNodeCalculator.reset();
            }
        }
        else
        {
            int numPrecalculated = clonedGame.lastPrecalculatedNode()->depth() - clonedGame.currentNode()->depth();
            if (numPrecalculated == 0)
            {                
                Assert(!mNodeCalculator);

                //
                // Clone the starting node
                //
                std::auto_ptr<GameStateNode> endNode = clonedGame.lastPrecalculatedNode()->clone();
                Assert(endNode->children().empty());
                Assert(endNode->depth() >= clonedGame.currentNode()->depth());


                //
                // Create the list of future blocks
                //
                BlockTypes futureBlocks;
                clonedGame.getFutureBlocksWithOffset(endNode->depth(), mSearchDepth, futureBlocks);


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
                Assert(mWorkerPool.stats().activeWorkerCount == 0);
                mNodeCalculator.reset(new NodeCalculator(endNode, futureBlocks, widths, mEvaluator->clone(), mWorkerPool));
                mNodeCalculator->start();
            }
            // else: we have plenty of precalculated nodes. Do nothing for know.
        }
    }


    ComputerPlayer::ComputerPlayer(const Protected<Game> & inProtectedGame,
                                   std::auto_ptr<Evaluator> inEvaluator,
                                   int inSearchDepth,
                                   int inSearchWidth) :
        mImpl(new ComputerPlayerImpl(inProtectedGame, inEvaluator, inSearchDepth, inSearchWidth))
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

} // namespace Tetris
