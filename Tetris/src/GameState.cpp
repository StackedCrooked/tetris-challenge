#include "Tetris/Config.h"
#include "Tetris/GameState.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
#include "Tetris/Assert.h"
#include <algorithm>
#include <vector>


namespace Tetris {


Stats::Stats() :
    mNumLines(0),
    mNumSingles(0),
    mNumDoubles(0),
    mNumTriples(0),
    mNumTetrises(0),
    mFirstOccupiedRow(0)
{
}


int Stats::score() const
{
    return 40 * mNumSingles + 100 * mNumDoubles + 300 * mNumTriples + 1200 * mNumTetrises;
}


int Stats::numLines() const
{
    return mNumLines;
}


int Stats::numSingles() const
{
    return mNumSingles;
}


int Stats::numDoubles() const
{
    return mNumDoubles;
}


int Stats::numTriples() const
{
    return mNumTriples;
}


int Stats::numTetrises() const
{
    return mNumTetrises;
}


int Stats::firstOccupiedRow() const
{
    return mFirstOccupiedRow;
}


// Use zero based index!
int Stats::numLines(size_t idx) const
{
    switch (idx)
    {
        case 0: return mNumSingles;
        case 1: return mNumDoubles;
        case 2: return mNumTriples;
        case 3: return mNumTetrises;
        default: throw std::invalid_argument("Invalid number of lines scored requested.");
    }
    return 0; // compiler happy
}


GameState::GameState(size_t inNumRows, size_t inNumColumns) :
    mGrid(inNumRows, inNumColumns, BlockType_Nil),
    mOriginalBlock(BlockType_L, Rotation(0), Row(0), Column(0)),
    mIsGameOver(false),
    mStats(),
    mQuality()
{
    mStats.mFirstOccupiedRow = inNumRows;
}


bool GameState::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
{
    const Grid & blockGrid(inBlock.grid());
    if (inColIdx < blockGrid.columnCount() &&
        (inRowIdx + blockGrid.rowCount()) < static_cast<size_t>(mStats.firstOccupiedRow()))
    {
        return true;
    }

    if (inRowIdx >= mGrid.rowCount())
    {
        return false;
    }

    for (size_t r = 0; r != blockGrid.rowCount(); ++r)
    {
        size_t rowIdx = inRowIdx + r;
        if (rowIdx >= mGrid.rowCount())
        {
            return false;
        }

        for (size_t c = 0; c != blockGrid.columnCount(); ++c)
        {
            size_t colIdx = inColIdx + c;
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


void GameState::forceUpdateStats()
{
    for (size_t r = 0; r != mGrid.rowCount(); ++r)
    {
        for (size_t c = 0; c != mGrid.columnCount(); ++c)
        {
            if (mGrid.get(r, c) != BlockType_Nil)
            {
                mStats.mFirstOccupiedRow = r;
                return;
            }
        }
    }
}


void GameState::solidifyBlock(const Block & inBlock)
{
    const Grid & grid = inBlock.grid();
    for (size_t r = 0; r != grid.rowCount(); ++r)
    {
        for (size_t c = 0; c != grid.columnCount(); ++c)
        {
            if (grid.get(r, c) != BlockType_Nil)
            {
                int gridRow = inBlock.row() + r;
                int gridCol = inBlock.column() + c;
                mGrid.set(gridRow, gridCol, inBlock.type());
                if (gridRow < mStats.mFirstOccupiedRow)
                {
                    mStats.mFirstOccupiedRow = gridRow;
                }
            }
        }
    }
}

    
void GameState::clearLines()
{
    int numLines = 0;
    unsigned int r = mOriginalBlock.row() + mOriginalBlock.rowCount() - 1;
    for (; r >= mStats.firstOccupiedRow(); --r)
    {
        unsigned int c = 0;
        bool line = true;
        for (; c < mGrid.columnCount(); ++c)
        {
            BlockType value = mGrid.get(r, c);
            if (numLines > 0)
            {
                mGrid.set(r + numLines, c, value);
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

    if (numLines > 0)
    {
        BlockType * gridBegin = const_cast<BlockType*>(&(mGrid.get(mStats.firstOccupiedRow(), 0)));
        memset(&gridBegin[0], 0, numLines * mGrid.columnCount());
    }
		
    Assert(mStats.mFirstOccupiedRow + numLines <= static_cast<int>(mGrid.rowCount()));
    mStats.mFirstOccupiedRow += numLines;
    mStats.mNumLines += numLines;

    switch (numLines)
    {
        case 0:
        {
            // Do nothing.
            break;
        }
        case 1:
        {
            mStats.mNumSingles++;
            break;
        }
        case 2:
        {
            mStats.mNumDoubles++;
            break;
        }
        case 3:
        {
            mStats.mNumTriples++;
            break;
        }
        case 4:
        {
            mStats.mNumTetrises++;
            break;
        }
        default:
        {
            throw std::logic_error("Invalid number of lines scored! ...?");
        }
    }
}


bool GameState::isGameOver() const
{
    return mIsGameOver;
}


const Grid & GameState::grid() const
{
    return mGrid;
}


Grid & GameState::grid()
{
    return mGrid;
}


int GameState::quality(const Evaluator & inEvaluator) const
{
    if (!mQuality.isInitialized())
    {
        mQuality.reset();
        mQuality.setScore(inEvaluator.evaluate(*this));
        mQuality.setInitialized(true);
    }
    return mQuality.score();
}


const Block & GameState::originalBlock() const
{
    return mOriginalBlock;
}


std::auto_ptr<GameState> GameState::commit(const Block & inBlock, GameOver inGameOver) const
{
    std::auto_ptr<GameState> result(new GameState(*this));
    result->mIsGameOver = inGameOver.get();
    result->mQuality.setInitialized(false);
    if (!inGameOver.get())
    {
        result->solidifyBlock(inBlock);
    }
    result->mOriginalBlock = inBlock;
    result->clearLines();
    return result;
}


const Stats & GameState::stats() const
{
    return mStats;
}


} // namespace Tetris

