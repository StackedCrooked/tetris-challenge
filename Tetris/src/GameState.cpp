#include "Tetris/GameState.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Futile/Assert.h"
#include <algorithm>
#include <vector>


namespace Tetris {


GameState::GameState(unsigned inRowCount, unsigned inColumnCount) :
    mGrid(inRowCount, inColumnCount, BlockType_Nil),
    mOriginalBlock(BlockType_L, Rotation(0), Row(0), Column(0)),
    mIsGameOver(false),
    mFirstOccupiedRow(inRowCount),
    mStats(),
    mTainted(false),
    mId(0)
{
}


GameState GameState::commit(const Block & inBlock) const
{
    GameState result(*this);
    result.mTainted = false;
    result.mOriginalBlock = inBlock;
    result.mIsGameOver = inBlock.row() == 0 && !result.checkPositionValid(inBlock, inBlock.row(), inBlock.column());
    result.mId = id() + 1;
    result.solidifyBlock(inBlock);
    if (!result.mIsGameOver)
    {
        result.clearLines();
    }
    return result;
}


bool GameState::checkPositionValid(const Block & inBlock, unsigned inRowIdx, unsigned inColIdx) const
{
    const Grid & blockGrid(inBlock.grid());

    if (inColIdx < blockGrid.columnCount() && (inRowIdx + blockGrid.rowCount()) < mFirstOccupiedRow)
    {
        return true;
    }

    if (inRowIdx >= mGrid.rowCount())
    {
        return false;
    }

    for (unsigned r = 0; r != blockGrid.rowCount(); ++r)
    {
        unsigned rowIdx = inRowIdx + r;
        if (rowIdx >= mGrid.rowCount())
        {
            return false;
        }

        for (unsigned c = 0; c != blockGrid.columnCount(); ++c)
        {
            unsigned colIdx = inColIdx + c;
            if (colIdx >= mGrid.columnCount())
            {
                return false;
            }

            if (blockGrid.get(r, c) && mGrid.get(rowIdx, colIdx))
            {
                return false;
            }
        }
    }
    return true;
}


void GameState::solidifyBlock(const Block & inBlock)
{
    const Grid & grid = inBlock.grid();
    for (unsigned r = 0; r != grid.rowCount(); ++r)
    {
        for (unsigned c = 0; c != grid.columnCount(); ++c)
        {
            if (grid.get(r, c) != BlockType_Nil)
            {
                unsigned gridRow = inBlock.row() + r;
                unsigned gridCol = inBlock.column() + c;
                mGrid.set(gridRow, gridCol, inBlock.type());
                if (gridRow < mFirstOccupiedRow)
                {
                    mFirstOccupiedRow = gridRow;
                }
            }
        }
    }
}


void GameState::clearLines()
{
    unsigned numLines = 0;
    int rowIndex = static_cast<int>(mOriginalBlock.row() + mOriginalBlock.rowCount()) - 1;
    for (; rowIndex >= static_cast<int>(mFirstOccupiedRow); --rowIndex)
    {
        unsigned colIndex = 0;
        bool line = true;
        for (; colIndex < mGrid.columnCount(); ++colIndex)
        {
            BlockType value = mGrid.get(rowIndex, colIndex);
            if (numLines > 0)
            {
                mGrid.set(rowIndex + numLines, colIndex, value);
            }

            if (!value)
            {
                line = false;
                if (numLines == 0)
                {
                    break;
                }
            }
        }
        if (line)
        {
            numLines++;
        }
    }

    Assert(numLines <= 4);
    if (numLines > 0)
    {
        BlockType * gridBegin = const_cast<BlockType*>(&(mGrid.get(mFirstOccupiedRow, 0)));
        memset(&gridBegin[0], 0, numLines * mGrid.columnCount());
    }

    Assert(mFirstOccupiedRow + numLines <= mGrid.rowCount());
    mFirstOccupiedRow += numLines;
    mStats = mStats.increment(numLines);
}


void GameState::setGrid(const Grid & inGrid)
{
    Assert(mGrid.rowCount() == inGrid.rowCount() && mGrid.columnCount() == inGrid.columnCount());
    mGrid = inGrid;
    mTainted = true;
    updateCache();
}


void GameState::updateCache()
{
    mFirstOccupiedRow = 0;
    while (mFirstOccupiedRow < mGrid.rowCount())
    {
        for (unsigned colIndex = 0; colIndex != mGrid.columnCount(); ++colIndex)
        {
            if (mGrid.get(mFirstOccupiedRow, colIndex) != BlockType_Nil)
            {
                return;
            }
        }
        mFirstOccupiedRow++;
    }
}


EvaluatedGameState::EvaluatedGameState(const GameState & inGameState, signed inQuality) :
    mGameState(inGameState),
    mQuality(inQuality)
{
}


EvaluatedGameState::~EvaluatedGameState()
{
}


const GameState & EvaluatedGameState::gameState() const
{
    return mGameState;
}


GameState & EvaluatedGameState::gameState()
{
    return mGameState;
}


} // namespace Tetris

