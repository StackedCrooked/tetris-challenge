#include "Tetris/ComputerPlayer.h"
#include "Tetris/MultithreadedNodeCalculator.h"
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
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Tetris/Assert.h"
#include <set>
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
        if (mNodeCalculator)
        {
            // Check if the computer player has finished.
            if (mNodeCalculator->status() != AbstractNodeCalculator::Status_Finished)
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
            if (numPrecalculated + mCurrentSearchDepth <= 3 * mMaxSearchDepth)
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
                // Create and start the NodeCalculator.
                //
                Assert(mWorkerPool->stats().activeWorkerCount == 0);
                mNodeCalculator.reset(new MultithreadedNodeCalculator(mWorkerPool, endNode, futureBlocks, widths, mEvaluator->clone()));
                mNodeCalculator->start();
            }
            // else: we have plenty of precalculated nodes. Do nothing for know.
        }
    }

} // namespace Tetris
