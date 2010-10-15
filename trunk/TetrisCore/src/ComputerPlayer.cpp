#include "Tetris/ComputerPlayer.h"
#include "Tetris/AISupport.h"
#include "Tetris/BlockMover.h"
#include "Tetris/ComputerPlayerMt.h"
#include "Tetris/Gravity.h"
#include "Tetris/Game.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/GameState.h"
#include "Tetris/BlockType.h"
#include "Tetris/WorkerPool.h"
#include "Tetris/Worker.h"
#include <boost/bind.hpp>


namespace Tetris
{
    
    ComputerPlayer::ComputerPlayer(const Protected<Game> & inProtectedGame) :
        mWorkerPool(new WorkerPool("ComputerPlayer", 6)),
        mProtectedGame(inProtectedGame),
        mGravity(new Gravity(mProtectedGame)),
        mBlockMover(new BlockMover(mProtectedGame, 20)),
        mEvaluator(new Balanced),
        mCurrentSearchDepth(0),
        mMaxSearchDepth(6),
        mSearchWidth(6)
    {
    }


    int ComputerPlayer::calculateRemainingTimeMs(Game & game) const
    {
        float numRemainingRows = static_cast<float>(game.currentNode()->state().stats().firstOccupiedRow() - (game.activeBlock().row() + 4));
        float numRowsPerSecond = mGravity->currentSpeed();
        float remainingTime = 1000 * numRemainingRows / numRowsPerSecond;
        float timeRequiredForMove = static_cast<float>(game.activeBlock().numRotations() + game.numColumns()) / static_cast<float>(mBlockMover->speed());
        return static_cast<int>(0.5 + remainingTime - timeRequiredForMove);
    }

    
    void ComputerPlayer::runImpl()
    {
        boost::scoped_ptr<Game> clonedGamePtr;
        {
            ScopedAtom<Game> wgame(mProtectedGame);
            clonedGamePtr.reset(wgame->clone().release());
        }
        Game & clonedGame(*clonedGamePtr);
        if (mMoveCalculator)
        {
            // Check if the computer player has finished.
            if (mMoveCalculator->status() != MoveCalculator::Status_Finished)
            {
                // Check if there is the danger of crashing the current block.
                if (clonedGame.numPrecalculatedMoves() == 0 && calculateRemainingTimeMs(clonedGame) < 1500)
                {
                    mMoveCalculator->stop();
                }
                // else: keep working.
            }
            else
            {
                NodePtr resultNode = mMoveCalculator->result();
                if (!resultNode->state().isGameOver())
                {
                    // The created node should follow the last precalculated one.
                    if (resultNode->depth() == clonedGame.lastPrecalculatedNode()->depth() + 1)
                    {
                        const int resultNodeDepth = resultNode->depth();

                        //
                        // TODO:
                        //
                        // Use *real* game object here and detect any sync errors.
                        //
                        //

                        // GameStateNode * endNode(clonedGame.lastPrecalculatedNode());
                        // Assert(endNode->depth() + 1 == resultNodeDepth);
                        // endNode->addChild(resultNode);

                    }
                    else
                    {
                        LogWarning("Computer is TOO SLOW!!");
                    }
                }
                else
                {
                    // Game over. Too bad.
                    mMoveCalculator.reset();
                    return;
                }                

                // Once the computer has finished it's job we destroy the object.
                mMoveCalculator.reset();
            }
        }
        else
        {
            int numPrecalculated = clonedGame.lastPrecalculatedNode()->depth() - clonedGame.currentNode()->depth();
            if (numPrecalculated + mCurrentSearchDepth <= 3 * mMaxSearchDepth)
            {
                Assert(!mMoveCalculator);

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
                clonedGame.getFutureBlocksWithOffset(endNode->depth(), mCurrentSearchDepth, futureBlocks);


                //
                // Fill the Widths vector (use the same width for each level).
                //
                Widths widths;
                for (size_t idx = 0; idx != futureBlocks.size(); ++idx)
                {
                    widths.push_back(mSearchWidth);
                }


                //
                // Create and start the ConcreteMoveCalculator.
                //
                Assert(mWorkerPool->stats().activeWorkerCount == 0);
                mMoveCalculator.reset(new MoveCalculatorMt(mWorkerPool, endNode, futureBlocks, widths, mEvaluator->clone()));
                mMoveCalculator->start();
            }
            // else: we have plenty of precalculated nodes. Do nothing for know.
        }
    }


    ConcreteMoveCalculator::ConcreteMoveCalculator(boost::shared_ptr<Worker> inWorker,
                                                   std::auto_ptr<GameStateNode> inNode,
                                                   const BlockTypes & inBlockTypes,
                                                   const Widths & inWidths,
                                                   std::auto_ptr<Evaluator> inEvaluator) :
        MoveCalculator(),
        mLayers(inBlockTypes.size()),
        mCompletedSearchDepth(0),
        mNode(inNode.release()),
        mBlockTypes(inBlockTypes),
        mWidths(inWidths),
        mEvaluator(inEvaluator.release()),
        mStatus(Status_Nil),
        mWorker(inWorker),
        mDestroyedInferiorChildren(false)
    {
        Assert(!mNode->state().isGameOver());
        Assert(mNode->children().empty());
        mNode->makeRoot(); // forget out parent node
    }


    ConcreteMoveCalculator::~ConcreteMoveCalculator()
    {
        mWorker->interruptAndClearQueue();
    }


    void ConcreteMoveCalculator::getLayerData(int inIndex, LayerData & outLayerData)
    {
        boost::mutex::scoped_lock lock(mLayersMutex);
        outLayerData = mLayers[inIndex];
    }


    int ConcreteMoveCalculator::getCurrentSearchDepth() const
    {
        boost::mutex::scoped_lock lock(mCompletedSearchDepthMutex);
        return mCompletedSearchDepth;
    }


    void ConcreteMoveCalculator::setCurrentSearchDepth(int inDepth)
    {
        boost::mutex::scoped_lock lock(mCompletedSearchDepthMutex);
        if (inDepth > mCompletedSearchDepth)
        {
            mCompletedSearchDepth = inDepth;
        }
    }


    int ConcreteMoveCalculator::getMaxSearchDepth() const
    {
        return mWidths.size();
    }


    NodePtr ConcreteMoveCalculator::result() const
    {
        Assert(status() == Status_Finished);

        boost::mutex::scoped_lock lock(mNodeMutex);
        
        // Impossible since the thread interruption point is
        // not triggered before search depth 1 is complete.
        Assert(!mNode->children().empty());

        size_t numChildren = mNode->children().size();

        // DestroyInferiorChildren should
        // have taken care of this.
        Assert(mDestroyedInferiorChildren);
        Assert(numChildren == 1);

        return *mNode->children().begin();
    }


    MoveCalculator::Status ConcreteMoveCalculator::status() const
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        return mStatus;
    }


    void ConcreteMoveCalculator::setStatus(Status inStatus)
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        mStatus = inStatus;
    }


    void ConcreteMoveCalculator::updateLayerData(size_t inIndex, NodePtr inNodePtr, size_t inCount)
    {
        boost::mutex::scoped_lock lock(mLayersMutex);
        LayerData & layerData = mLayers[inIndex];
        layerData.mNumItems += inCount;
        if (!layerData.mBestChild || inNodePtr->state().quality(inNodePtr->qualityEvaluator()) > layerData.mBestChild->state().quality(inNodePtr->qualityEvaluator()))
        {
            layerData.mBestChild = inNodePtr;
        }
    }


    void ConcreteMoveCalculator::populateNodesRecursively(
        NodePtr ioNode,
        const BlockTypes & inBlockTypes,
        const Widths & inWidths,
        size_t inIndex,
        size_t inMaxIndex)
    {

        // We want to at least perform a depth-1 search.
        if (inIndex > 0)
        {
            boost::this_thread::interruption_point();
        }

        //
        // Check stop conditions
        //
        if (inIndex > inMaxIndex || inIndex >= inBlockTypes.size())
        {
            return;
        }


        if (ioNode->state().isGameOver())
        {
            // Game over state has no children.
            return;
        }


        //
        // Generate the child nodes.
        //
        // It is possible that the nodes were already generated at this depth.
        // If that is the case then we immediately jump to the recursive call below.
        //
        ChildNodes generatedChildNodes = ioNode->children();
        if (generatedChildNodes.empty())
        {
            generatedChildNodes = ChildNodes(GameStateComparisonFunctor(mEvaluator->clone()));
            GenerateOffspring(ioNode, inBlockTypes[inIndex], *mEvaluator, generatedChildNodes);

            int count = 0;
            ChildNodes::iterator it = generatedChildNodes.begin(), end = generatedChildNodes.end();
            while (count < inWidths[inIndex] && it != end)
            {
                ioNode->addChild(*it);
                ++count;
                ++it;
            }
            updateLayerData(inIndex, *ioNode->children().begin(), count);
        }


        //
        // Recursive call on each child node.
        //
        if (inIndex < inMaxIndex)
        {
            for (ChildNodes::iterator it = generatedChildNodes.begin(); it != generatedChildNodes.end(); ++it)
            {
                NodePtr child = *it;
                populateNodesRecursively(child, inBlockTypes, inWidths, inIndex + 1, inMaxIndex);
            }
        }
    }


    void ConcreteMoveCalculator::markTreeRowAsFinished(size_t inIndex)
    {
        setCurrentSearchDepth(inIndex + 1);
        boost::mutex::scoped_lock lock(mLayersMutex);
        mLayers[inIndex].mFinished = true;
    }


    void ConcreteMoveCalculator::populate()
    {
        try
        {
            // The nodes are populated using a "Iterative deepening" algorithm.
            // See: http://en.wikipedia.org/wiki/Iterative_deepening_depth-first_search for more information.
            size_t targetDepth = 0;
            while (targetDepth < mBlockTypes.size())
            {
                boost::mutex::scoped_lock lock(mNodeMutex);
                populateNodesRecursively(mNode, mBlockTypes, mWidths, 0, targetDepth);
                markTreeRowAsFinished(targetDepth);
                targetDepth++;
            }
        }
        catch (const boost::thread_interrupted &)
        {
            // Task was interrupted. Ok.
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << "Exception caught in ConcreteMoveCalculator::populate(). Detail: " << inException.what());
        }
    }


    void ConcreteMoveCalculator::destroyInferiorChildren()
    {
        Assert(!mDestroyedInferiorChildren);
        size_t reachedDepth = getCurrentSearchDepth();
        Assert(reachedDepth >= 1);

        // We use the 'best child' from this search depth.
        // The path between the start node and this best
        // child will be the list of precalculated nodes.
        boost::mutex::scoped_lock layersLock(mLayersMutex);
        boost::mutex::scoped_lock nodeLock(mNodeMutex);
        Assert((reachedDepth - 1) < mLayers.size());
        CarveBestPath(mNode, mLayers[reachedDepth - 1].mBestChild);
        Assert(mNode->children().size() == 1);
        mDestroyedInferiorChildren = true;
    }


    void ConcreteMoveCalculator::stop()
    {
        if (status() == Status_Started || status() == Status_Working)
        {
            setStatus(Status_Stopped);
            mWorker->interruptAndClearQueue();
        }
    }


    void ConcreteMoveCalculator::startImpl()
    {
        // Thread entry point has try/catch block
        try
        {
            setStatus(Status_Working);
            populate();
            destroyInferiorChildren();
        }
        catch (const std::exception & inException)
        {
            LogError(MakeString() << inException.what());
        }
        setStatus(Status_Finished);
    }


    void ConcreteMoveCalculator::start()
    {
        boost::mutex::scoped_lock lock(mStatusMutex);
        Assert(mStatus == Status_Nil);
        mWorker->schedule(boost::bind(&ConcreteMoveCalculator::startImpl, this));
        mStatus = Status_Started;
    }

} // namespace Tetris
