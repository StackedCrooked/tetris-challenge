#include "Tetris/Config.h"
#include "Tetris/GameState.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Tetris/Assert.h"
#include <algorithm>
#include <vector>


namespace Tetris {


GameState::GameState(size_t inNumRows, size_t inNumColumns) :
    mGrid(inNumRows, inNumColumns, BlockType_Nil),
    mOriginalBlock(BlockType_L, Rotation(0), Row(0), Column(0)),
    mIsGameOver(false),
    mFirstOccupiedRow(inNumRows),
    mNumLines(0),
    mNumSingles(0),
    mNumDoubles(0),
    mNumTriples(0),
    mNumTetrises(0)
{
}


bool GameState::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
{
    const Grid & blockGrid(inBlock.grid());
    if (inColIdx < blockGrid.columnCount() &&
        (inRowIdx + blockGrid.rowCount()) < static_cast<size_t>(mFirstOccupiedRow))
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
    int numLines = 0;
    int r = mOriginalBlock.row() + mOriginalBlock.rowCount() - 1;
    for (; r >= mFirstOccupiedRow; --r)
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
        BlockType * gridBegin = const_cast<BlockType*>(&(mGrid.get(mFirstOccupiedRow, 0)));
        memset(&gridBegin[0], 0, numLines * mGrid.columnCount());
    }

    Assert(mFirstOccupiedRow + numLines <= static_cast<int>(mGrid.rowCount()));
    mFirstOccupiedRow += numLines;
    mNumLines += numLines;

    switch (numLines)
    {
        case 0:
        {
            // Do nothing.
            break;
        }
        case 1:
        {
            mNumSingles++;
            break;
        }
        case 2:
        {
            mNumDoubles++;
            break;
        }
        case 3:
        {
            mNumTriples++;
            break;
        }
        case 4:
        {
            mNumTetrises++;
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


void GameState::setGrid(const Grid & inGrid)
{
    mGrid = inGrid;
    updateCache();
}


void GameState::updateCache()
{
    mFirstOccupiedRow = mGrid.rowCount();
    for (size_t r = 0; r != mGrid.rowCount(); ++r)
    {
        for (size_t c = 0; c != mGrid.columnCount(); ++c)
        {
            if (mGrid.get(r, c) != BlockType_Nil)
            {
                mFirstOccupiedRow = r;
                LogInfo(MakeString() << "mFirstOccupiedRow is now " << mFirstOccupiedRow);
                return;
            }
        }
    }
}


int GameState::score() const
{
    // Same values as Tetris on the Gameboy.
    return   40 * mNumSingles +
            100 * mNumDoubles +
            300 * mNumTriples +
           1200 * mNumTetrises;
}

const Block & GameState::originalBlock() const
{
    return mOriginalBlock;
}


std::auto_ptr<GameState> GameState::commit(const Block & inBlock, GameOver inGameOver) const
{
    std::auto_ptr<GameState> result(new GameState(*this));
    result->mIsGameOver = inGameOver.get();
    if (!inGameOver.get())
    {
        result->solidifyBlock(inBlock);
    }
    result->mOriginalBlock = inBlock;
    result->clearLines();
    return result;
}


EvaluatedGameState::EvaluatedGameState(GameState * inGameState, int inQuality) :
    mGameState(inGameState),
    mQuality(inQuality)
{
}


EvaluatedGameState::~EvaluatedGameState()
{
    delete mGameState;
}


const GameState & EvaluatedGameState::gameState() const
{
    return *mGameState;
}


GameState & EvaluatedGameState::gameState()
{
    return *mGameState;
}


int EvaluatedGameState::quality() const
{
    return mQuality;
}


} // namespace Tetris

