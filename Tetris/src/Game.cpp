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
#include <stdexcept>


namespace Tetris {


extern const int cMaxLevel;


HumanGame::HumanGame(size_t inNumRows, size_t inNumCols) :
    AbstractGame(inNumRows, inNumCols),
    mGameState(new GameState(inNumRows, inNumCols))
{
}


GameState & HumanGame::getGameState()
{
    if (!mGameState.get())
    {
        throw std::logic_error("Null pointer deref: mGameState");
    }
    return *mGameState;
}


const GameState & HumanGame::getGameState() const
{
    if (!mGameState.get())
    {
        throw std::logic_error("Null pointer deref: mGameState");
    }
    return *mGameState;
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


bool HumanGame::move(Direction inDirection)
{
    if (isGameOver())
    {
        return false;
    }

    Block & block = *mActiveBlock;
    size_t newRow = block.row() + GetRowDelta(inDirection);
    size_t newCol = block.column() + GetColumnDelta(inDirection);
    if (getGameState().checkPositionValid(block, newRow, newCol))
    {
        block.setRow(newRow);
        block.setColumn(newCol);
        setDirty();
        return true;
    }

    if (inDirection != Direction_Down)
    {
        // Do nothing
        return false;
    }

    mGameState.reset(mGameState->commit(block, GameOver(block.row() == 0)).release());

    mCurrentBlockIndex++;
    supplyBlocks();
    mActiveBlock.reset(CreateDefaultBlock(mBlocks[mCurrentBlockIndex], mNumColumns).release());

    setDirty();
    return false;
}


ComputerGame::ComputerGame(size_t inNumRows, size_t inNumCols) :
    AbstractGame(inNumRows, inNumCols),
    mCurrentNode(GameStateNode::CreateRootNode(inNumRows, inNumCols).release())
{
}


GameState & ComputerGame::getGameState()
{
    return mCurrentNode->state();
}


const GameState & ComputerGame::getGameState() const
{
    return mCurrentNode->state();
}


void ComputerGame::setCurrentNode(NodePtr inCurrentNode)
{
    Assert(inCurrentNode->depth() == mCurrentNode->depth() + 1);

    mCurrentNode = inCurrentNode;
    mCurrentBlockIndex = mCurrentNode->depth();
    supplyBlocks();

    mActiveBlock.reset(CreateDefaultBlock(mBlocks[mCurrentBlockIndex], mNumColumns).release());
    setDirty();
}


size_t ComputerGame::numPrecalculatedMoves() const
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


void ComputerGame::clearPrecalculatedNodes()
{
    mCurrentNode->children().clear();
}


const GameStateNode * ComputerGame::currentNode() const
{
    return mCurrentNode.get();
}


const GameStateNode * ComputerGame::lastPrecalculatedNode() const
{
    return mCurrentNode->endNode();
}


void ComputerGame::appendPrecalculatedNode(NodePtr inNode)
{
    mCurrentNode->endNode()->addChild(inNode);
}


bool ComputerGame::navigateNodeDown()
{
    if (mCurrentNode->children().empty())
    {
        return false;
    }

    NodePtr nextNode = *mCurrentNode->children().begin();
    Assert(nextNode->depth() == mCurrentNode->depth() + 1);
    setCurrentNode(nextNode);
    setDirty();
    return true;
}


bool ComputerGame::move(Direction inDirection)
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
        setDirty();
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
    setDirty();
    return false;
}



AbstractGame::AbstractGame(size_t inNumRows, size_t inNumColumns) :
    mNumRows(inNumRows),
    mNumColumns(inNumColumns),
    mActiveBlock(),
    mBlockFactory(new BlockFactory),
    mBlocks(),
    mCurrentBlockIndex(0),
    mOverrideLevel(-1),
    mDirty(true)
{
    if (mBlocks.empty())
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
    mActiveBlock.reset(CreateDefaultBlock(mBlocks.front(), inNumColumns).release());
}


AbstractGame::~AbstractGame()
{
}


std::auto_ptr<Block> AbstractGame::CreateDefaultBlock(BlockType inBlockType, size_t inNumColumns)
{
    return std::auto_ptr<Block>(
        new Block(inBlockType,
                    Rotation(0),
                    Row(0),
                    Column(DivideByTwo(inNumColumns - GetGrid(GetBlockIdentifier(inBlockType, 0)).columnCount()))));
}


bool AbstractGame::checkDirty()
{
    boost::mutex::scoped_lock lock(mDirtyMutex);
    if (mDirty)
    {
        mDirty = false;
        return true;
    }
    return false;
}


void AbstractGame::setDirty()
{
    boost::mutex::scoped_lock lock(mDirtyMutex);
    mDirty = true;
}


void AbstractGame::supplyBlocks() const
{
    while (mCurrentBlockIndex >= mBlocks.size())
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
}


bool AbstractGame::isGameOver() const
{
    return getGameState().isGameOver();
}


int AbstractGame::rowCount() const
{
    return mNumRows;
}


int AbstractGame::columnCount() const
{
    return mNumColumns;
}


void AbstractGame::reserveBlocks(size_t inCount)
{
    while (mBlocks.size() < inCount)
    {
        mBlocks.push_back(mBlockFactory->getNext());
    }
}


const Block & AbstractGame::activeBlock() const
{
    supplyBlocks();
    return *mActiveBlock;
}


const Grid & AbstractGame::gameGrid() const
{
    return getGameState().grid();
}


void AbstractGame::getFutureBlocks(size_t inCount, BlockTypes & outBlocks) const
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


void AbstractGame::getFutureBlocksWithOffset(size_t inOffset, size_t inCount, BlockTypes & outBlocks) const
{
    if (!mBlockFactory && (mBlocks.size() < inOffset + inCount))
    {
        throw std::runtime_error("This is a cloned HumanGame object and its number of future blocks is depleted.");
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


size_t AbstractGame::currentBlockIndex() const
{
    return mCurrentBlockIndex;
}


bool AbstractGame::rotate()
{
    if (isGameOver())
    {
        return false;
    }

    Block & block = *mActiveBlock;
    size_t oldRotation = block.rotation();
    block.rotate();
    if (!getGameState().checkPositionValid(block, block.row(), block.column()))
    {
        block.setRotation(oldRotation);
        return false;
    }
    setDirty();
    return true;
}


void AbstractGame::drop()
{
    while (move(Direction_Down))
    {
        // Keep going.
        setDirty();
    }
}


int AbstractGame::level() const
{
    if (mOverrideLevel < 0)
    {
        int level = getGameState().numLines() / 10;
        return std::min<int>(level, cMaxLevel);
    }
    else
    {
        return mOverrideLevel;
    }
}


void AbstractGame::setLevel(int inLevel)
{
    mOverrideLevel = inLevel;
    setDirty();
}


} // namespace Tetris
