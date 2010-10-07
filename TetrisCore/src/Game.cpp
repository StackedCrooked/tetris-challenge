#include "Tetris/Game.h"
#include "Tetris/Assert.h"
#include "Tetris/GameState.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/Logger.h"
#include "Tetris/Utilities.h"


namespace Tetris
{

    std::auto_ptr<Block> CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns)
    {
        return std::auto_ptr<Block>(new Block(inBlockType,
                                              Rotation(0),
                                              Row(0),
                                              Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).numColumns()))));
    }


    Game::Game(size_t inNumRows, size_t inNumColumns) :
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


    Game::~Game()
    {
    }


    int Game::numRows() const
    {
        return mNumRows;
    }


    int Game::numColumns() const
    {
        return mNumColumns;
    }


    void Game::reserveBlocks(size_t inCount)
    {
        while (mBlocks.size() < inCount)
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }
    }


    bool Game::isGameOver() const
    {
        return mCurrentNode->state().isGameOver();
    }


    void Game::supplyBlocks() const
    {
        while (mCurrentBlockIndex >= mBlocks.size())
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }
    }


    const Block & Game::activeBlock() const
    {
        supplyBlocks();
        return *mActiveBlock;
    }


    Block & Game::activeBlock()
    {
        supplyBlocks();
        return *mActiveBlock;
    }


    void Game::getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const
    {

        // Make sure we have all blocks we need.
        while (mBlocks.size() < mCurrentBlockIndex + inCount)
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }

        for (size_t idx = 0; idx < inCount; ++idx)
        {
            outBlocks.push_back(mBlocks[mCurrentBlockIndex + idx]);
        }
    }


    void Game::getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const
    {
        // Make sure we have all blocks we need.
        while (mBlocks.size() < inOffset + inCount)
        {
            mBlocks.push_back(mBlockFactory->getNext());
        }

        for (size_t idx = 0; idx < inCount; ++idx)
        {
            outBlocks.push_back(mBlocks[inOffset + idx]);
        }
    }


    size_t Game::numPrecalculatedMoves() const
    {
        size_t countMovesAhead = 0;
        const GameStateNode * tmp = mCurrentNode.get();
        while (!tmp->children().empty())
        {
            tmp = tmp->children().begin()->get();
            countMovesAhead++;
        }
        return countMovesAhead;
    }


    GameStateNode * Game::currentNode()
    {
        return mCurrentNode.get();
    }


    const GameStateNode * Game::currentNode() const
    {
        return mCurrentNode.get();
    }


    const GameStateNode * Game::lastPrecalculatedNode() const
    {
        return mCurrentNode->endNode();
    }


    GameStateNode * Game::lastPrecalculatedNode()
    {
        return mCurrentNode->endNode();
    }


    void Game::setCurrentNode(NodePtr inCurrentNode)
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


    bool Game::navigateNodeDown()
    {
        if (mCurrentNode->children().empty())
        {
            return false;
        }

        NodePtr nextNode = *mCurrentNode->children().begin();
        Assert(nextNode->depth() == mCurrentNode->depth() + 1);
        setCurrentNode(nextNode);
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
        if (mCurrentNode->state().checkPositionValid(block, newRow, newCol))
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
        if (!mCurrentNode->children().empty())
        {
            const GameStateNode & precalculatedChild = **mCurrentNode->children().begin();
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
        if (!mCurrentNode->children().empty())
        {
            LogWarning("Existing children when commiting a block. They will be deleted.");
            mCurrentNode->children().clear();
        }

        // Actually commit the block
        NodePtr child(new GameStateNode(mCurrentNode,
                                        mCurrentNode->state().commit(block, GameOver(block.row() == 0)),
                                        CreatePoly<Evaluator, Balanced>()));
        mCurrentNode->addChild(child);
        setCurrentNode(child);
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
        if (!mCurrentNode->state().checkPositionValid(block, block.row(), block.column()))
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
