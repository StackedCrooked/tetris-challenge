#include "Tetris/Game.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparisonFunctor.h"
#include "Tetris/GameState.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/BlockFactory.h"
#include "Tetris/Block.h"
#include "Tetris/Direction.h"
#include "Tetris/Utilities.h"
#include "Tetris/Logging.h"
#include "Tetris/Assert.h"
#include <set>
#include <boost/scoped_ptr.hpp>


namespace Tetris
{
    class GameImpl
    {
    public:
        GameImpl(size_t inNumRows, size_t inNumColumns) :
            mNumRows(inNumRows),
            mNumColumns(inNumColumns),
            mCurrentNode(GameStateNode::CreateRootNode(inNumRows, inNumColumns).release()),
            mActiveBlock(),
            mBlockFactory(new BlockFactory),
            mBlocks(),
            mCurrentBlockIndex(0)
        {
            if (mBlocks.empty())
            {
                mBlocks.push_back(mBlockFactory->getNext());
            }
            mActiveBlock.reset(CreateDefaultBlock(mBlocks.front(), inNumColumns).release());
        }

        GameImpl(const GameImpl & rhs) :
            mNumRows(rhs.mNumRows),
            mNumColumns(rhs.mNumColumns),
            mCurrentNode(rhs.mCurrentNode->clone()),
            mActiveBlock(new Block(*rhs.mActiveBlock)),
            mBlockFactory(), // YES! Because we must be CERTAIN that getFutureBlocks() will always return the same result.
                             //      The clone method will provide us with 100 precalculated blocks. After that this clone
                             //      becomes invalid. A runtime_exception will thrown the next time mBlockFactory will be
                             //      accessed.
            mBlocks(rhs.mBlocks),
            mCurrentBlockIndex(rhs.mCurrentBlockIndex)
        {
        }

        // Assignment is now allowed.
        GameImpl & operator=(const GameImpl &);

    private:
        static std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns)
        {
            return std::auto_ptr<Block>(
                new Block(inBlockType,
                          Rotation(0),
                          Row(0),
                          Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).numColumns()))));
        }

        void setCurrentNode(NodePtr inCurrentNode)
        {            
            Assert(inCurrentNode->depth() == mCurrentNode->depth() + 1);

            mCurrentNode = inCurrentNode;
            size_t oldBlockIndex = mCurrentBlockIndex;
            mCurrentBlockIndex = mCurrentNode->depth();
            supplyBlocks();
            mActiveBlock.reset(CreateDefaultBlock(mBlocks[mCurrentBlockIndex], mNumColumns).release());
            if (!mCurrentNode->children().empty())
            {
                GameState & state = (*mCurrentNode->children().begin())->state();
                Assert(mActiveBlock->type() == state.originalBlock().type());
            }
        }

        void supplyBlocks() const
        {            
            if (!mBlockFactory && (mCurrentBlockIndex >= mBlocks.size()))
            {
                throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
            }

            while (mCurrentBlockIndex >= mBlocks.size())
            {
                mBlocks.push_back(mBlockFactory->getNext());
            }
        }

        friend class Game;
        size_t mNumRows;
        size_t mNumColumns;
        NodePtr mCurrentNode;
        boost::scoped_ptr<Block> mActiveBlock;
        boost::scoped_ptr<BlockFactory> mBlockFactory;
        mutable BlockTypes mBlocks;
        size_t mCurrentBlockIndex;
    };


    Game::Game(size_t inNumRows, size_t inNumColumns) :
        mImpl(new GameImpl(inNumRows, inNumColumns))
    {
    }
    
    
    Game::Game(const Game & inGame) :
        mImpl(new GameImpl(*inGame.mImpl))
    {
    }


    Game::~Game()
    {
        delete mImpl;
        mImpl = 0;
    }


    std::auto_ptr<Game> Game::clone()
    {
        if (!mImpl->mBlockFactory)
        {
            throw std::runtime_error("You cannot clone a Game that was already cloned.");
        }


        // Make sure we have 100 blocks from the factory.
        // This ensures 
        while (mImpl->mCurrentBlockIndex + 100 > mImpl->mBlocks.size())
        {
            mImpl->mBlocks.push_back(mImpl->mBlockFactory->getNext());
        }
        return std::auto_ptr<Game>(new Game(*this));
    }


    GameStateNode * FindCurrentNode(GameStateNode * inRootNode, size_t inDepth)
    {
        if (inDepth == 0)
        {
            return inRootNode;
        }

        if (inRootNode->children().size() == 1)
        {
            return FindCurrentNode(inRootNode->children().begin()->get(), inDepth - 1);
        }
        else if (inRootNode->children().empty())
        {
            throw std::logic_error("There are no nodes at the requested depth at all!");
        }
        else
        {
            throw std::logic_error("Failed to find the current node. Childnodes require cleanup first.");
        }
    }


    int Game::numRows() const
    {
        return mImpl->mNumRows;
    }


    int Game::numColumns() const
    {
        return mImpl->mNumColumns;
    }


    void Game::reserveBlocks(size_t inCount)
    {
        if (!mImpl->mBlockFactory && (mImpl->mBlocks.size() < inCount))
        {
            throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
        }

        while (mImpl->mBlocks.size() < inCount)
        {
            mImpl->mBlocks.push_back(mImpl->mBlockFactory->getNext());
        }
    }


    bool Game::isGameOver() const
    {
        return mImpl->mCurrentNode->state().isGameOver();
    }


    const Block & Game::activeBlock() const
    {
        mImpl->supplyBlocks();
        return *mImpl->mActiveBlock;
    }


    Block & Game::activeBlock()
    {
        mImpl->supplyBlocks();
        return *mImpl->mActiveBlock;
    }


    void Game::getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const
    {
        if (!mImpl->mBlockFactory && (mImpl->mBlocks.size() < mImpl->mCurrentBlockIndex + inCount))
        {
            throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
        }

        // Make sure we have all blocks we need.
        while (mImpl->mBlocks.size() < mImpl->mCurrentBlockIndex + inCount)
        {
            mImpl->mBlocks.push_back(mImpl->mBlockFactory->getNext());
        }

        for (size_t idx = 0; idx < inCount; ++idx)
        {
            outBlocks.push_back(mImpl->mBlocks[mImpl->mCurrentBlockIndex + idx]);
        }
    }


    void Game::getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const
    {        
        if (!mImpl->mBlockFactory && (mImpl->mBlocks.size() < inOffset + inCount))
        {
            throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
        }


        // Make sure we have all blocks we need.
        while (mImpl->mBlocks.size() < inOffset + inCount)
        {
            mImpl->mBlocks.push_back(mImpl->mBlockFactory->getNext());
        }

        for (size_t idx = 0; idx < inCount; ++idx)
        {
            outBlocks.push_back(mImpl->mBlocks[inOffset + idx]);
        }
    }


    size_t Game::currentBlockIndex() const
    {
        return mImpl->mCurrentBlockIndex;
    }


    size_t Game::numPrecalculatedMoves() const
    {
        size_t countMovesAhead = 0;
        const GameStateNode * tmp = mImpl->mCurrentNode.get();
        while (!tmp->children().empty())
        {
            tmp = tmp->children().begin()->get();
            countMovesAhead++;
        }
        return countMovesAhead;
    }


    void Game::clearPrecalculatedNodes()
    {
        mImpl->mCurrentNode->children().clear();
    }


    GameStateNode * Game::currentNode()
    {
        return mImpl->mCurrentNode.get();
    }


    const GameStateNode * Game::currentNode() const
    {
        return mImpl->mCurrentNode.get();
    }


    const GameStateNode * Game::lastPrecalculatedNode() const
    {
        return mImpl->mCurrentNode->endNode();
    }


    GameStateNode * Game::lastPrecalculatedNode()
    {
        return mImpl->mCurrentNode->endNode();
    }


    bool Game::navigateNodeDown()
    {
        if (mImpl->mCurrentNode->children().empty())
        {
            return false;
        }

        NodePtr nextNode = *mImpl->mCurrentNode->children().begin();
        Assert(nextNode->depth() == mImpl->mCurrentNode->depth() + 1);
        mImpl->setCurrentNode(nextNode);
        return true;
    }


    static int GetRowDelta(Direction inDirection)
    {
        switch (inDirection)
        {
            case Direction_Up:
            {
                return -1;
            }
            case Direction_Down:
            {
                return 1;
            }
            default:
            {
                return 0;
            }
        }
    }


    static int GetColumnDelta(Direction inDirection)
    {
        switch (inDirection)
        {
            case Direction_Left:
            {
                return -1;
            }
            case Direction_Right:
            {
                return 1;
            }
            default:
            {
                return 0;
            }
        }
    }


    bool Game::move(Direction inDirection)
    {
        if (isGameOver())
        {
            return false;
        }

        Block & block = activeBlock();
        size_t newRow = block.row() + GetRowDelta(inDirection);
        size_t newCol = block.column() + GetColumnDelta(inDirection);
        if (mImpl->mCurrentNode->state().checkPositionValid(block, newRow, newCol))
        {
            block.setRow(newRow);
            block.setColumn(newCol);
            return true;
        }

        if (inDirection != Direction_Down)
        {
            // Do nothing
            return false;
        }


        //
        // We can't move the block down any further => we hit the bottom => commit the block
        //

        // First check if we already have a matching precalculated block.
        if (!mImpl->mCurrentNode->children().empty())
        {
            const GameStateNode & precalculatedChild = **mImpl->mCurrentNode->children().begin();
            const Block & nextBlock = precalculatedChild.state().originalBlock();
            Assert(nextBlock.type() == block.type());
            if (block.column() == nextBlock.column() &&
                    block.rotation() == nextBlock.rotation())
            {
                return navigateNodeDown();
            }
        }

        // We don't have a matching precalculating block.
        // => Erase any existing children (should not happen)
        if (!mImpl->mCurrentNode->children().empty())
        {
            LogWarning("Existing children when commiting a block. They will be deleted.");
            mImpl->mCurrentNode->children().clear();
        }

        // Actually commit the block
        NodePtr child(new GameStateNode(mImpl->mCurrentNode,
                                        mImpl->mCurrentNode->state().commit(block, GameOver(block.row() == 0)),
                                        CreatePoly<Evaluator, Balanced>()));
        mImpl->mCurrentNode->addChild(child);
        mImpl->setCurrentNode(child);
        return true;
    }


    bool Game::rotate()
    {
        if (isGameOver())
        {
            return false;
        }

        Block & block = activeBlock();
        size_t oldRotation = block.rotation();
        block.rotate();
        if (!mImpl->mCurrentNode->state().checkPositionValid(block, block.row(), block.column()))
        {
            block.setRotation(oldRotation);
            return false;
        }
        return true;
    }


    void Game::drop()
    {
        while (move(Direction_Down));
    }


} // namespace Tetris
