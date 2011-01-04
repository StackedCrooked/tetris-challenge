#include "Tetris/Config.h"
#include "Tetris/Game.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/GameStateComparator.h"
#include "Tetris/GameState.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Block.h"
#include "Tetris/Utilities.h"
#include "Tetris/Logging.h"
#include "Tetris/Assert.h"
#include <algorithm>
#include <set>


namespace Tetris {


extern const int cMaxLevel;


Game::Game(size_t inNumRows, size_t inNumColumns) :
    mNumRows(inNumRows),
    mNumColumns(inNumColumns),
    mCurrentNode(GameStateNode::CreateRootNode(inNumRows, inNumColumns).release()),
    mActiveBlock(),
    mBlockFactory(new BlockFactory),
    mBlocks(),
    mCurrentBlockIndex(0),
    mOverrideLevel(-1)
{
    if (mBlocks.empty())
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
    mActiveBlock.reset(CreateDefaultBlock(mBlocks.front(), inNumColumns).release());
}


std::auto_ptr<Block> Game::CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns)
{
    return std::auto_ptr<Block>(
        new Block(inBlockType,
                    Rotation(0),
                    Row(0),
                    Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount()))));
}


void Game::setCurrentNode(NodePtr inCurrentNode)
{            
    Assert(inCurrentNode->depth() == mCurrentNode->depth() + 1);

    mCurrentNode = inCurrentNode;
    mCurrentBlockIndex = mCurrentNode->depth();
    supplyBlocks();

    mActiveBlock.reset(CreateDefaultBlock(mBlocks[mCurrentBlockIndex], mNumColumns).release());
}


void Game::supplyBlocks() const
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


int Game::rowCount() const
{
    return mNumRows;
}


int Game::columnCount() const
{
    return mNumColumns;
}


void Game::reserveBlocks(size_t inCount)
{
    if (!mBlockFactory && (mBlocks.size() < inCount))
    {
        throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
    }

    while (mBlocks.size() < inCount)
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
}


bool Game::isGameOver() const
{
    return mCurrentNode->state().isGameOver();
}


const Block & Game::activeBlock() const
{
    supplyBlocks();
    return *mActiveBlock;
}



const Grid & Game::gameGrid() const
{
    return mCurrentNode->state().grid();
}


void Game::getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const
{
    if (!mBlockFactory && (mBlocks.size() < mCurrentBlockIndex + inCount))
    {
        throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
    }

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
    if (!mBlockFactory && (mBlocks.size() < inOffset + inCount))
    {
        throw std::runtime_error("This is a cloned Game object and its number of future blocks is depleted.");
    }


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


size_t Game::currentBlockIndex() const
{
    return mCurrentBlockIndex;
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


void Game::clearPrecalculatedNodes()
{
    mCurrentNode->children().clear();
}


const GameStateNode * Game::currentNode() const
{
    return mCurrentNode.get();
}


const GameStateNode * Game::lastPrecalculatedNode() const
{
    return mCurrentNode->endNode();
}


void Game::appendPrecalculatedNode(NodePtr inNode)
{
    mCurrentNode->endNode()->addChild(inNode);
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

    Block & block = *mActiveBlock;
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
    return false;
}


bool Game::rotate()
{
    if (isGameOver())
    {
        return false;
    }

    Block & block = *mActiveBlock;
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
    while (move(Direction_Down))
    {
        // Keep going.
    }
}


int Game::level() const
{
    if (mOverrideLevel < 0)
    {
        int level = mCurrentNode->state().numLines() / 10;
        return std::min<int>(level, cMaxLevel);
    }
    else
    {
        return mOverrideLevel;
    }
}


void Game::setLevel(int inLevel)
{
    mOverrideLevel = inLevel;
}


} // namespace Tetris
