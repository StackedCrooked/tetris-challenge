#include "Tetris/Config.h"
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


GameState::GameState(std::size_t inNumRows, std::size_t inNumColumns) :
    mGrid(inNumRows, inNumColumns, BlockType_Nil),
    mOriginalBlock(BlockType_L, Rotation(0), Row(0), Column(0)),
    mIsGameOver(false),
    mFirstOccupiedRow(inNumRows),
    mNumLines(0),
    mNumSingles(0),
    mNumDoubles(0),
    mNumTriples(0),
    mNumTetrises(0),
    mTainted(false)
{
}


bool GameState::checkPositionValid(const Block& inBlock, std::size_t inRowIdx, std::size_t inColIdx) const
{
    const Grid& blockGrid(inBlock.grid());

    if (inColIdx < blockGrid.columnCount() && (inRowIdx + blockGrid.rowCount()) < mFirstOccupiedRow)
    {
        return true;
    }

    if (inRowIdx >= mGrid.rowCount())
    {
        return false;
    }

    for (std::size_t r = 0; r != blockGrid.rowCount(); ++r)
    {
        std::size_t rowIdx = inRowIdx + r;
        if (rowIdx >= mGrid.rowCount())
        {
            return false;
        }

        for (std::size_t c = 0; c != blockGrid.columnCount(); ++c)
        {
            std::size_t colIdx = inColIdx + c;
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


void GameState::solidifyBlock(const Block& inBlock)
{
    const Grid& grid = inBlock.grid();
    for (std::size_t r = 0; r != grid.rowCount(); ++r)
    {
        for (std::size_t c = 0; c != grid.columnCount(); ++c)
        {
            if (grid.get(r, c) != BlockType_Nil)
            {
				std::size_t gridRow = inBlock.row() + r;
				std::size_t gridCol = inBlock.column() + c;
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
	std::size_t numLines = 0;
	int rowIndex = mOriginalBlock.row() + mOriginalBlock.rowCount() - 1;
    for (; rowIndex >= static_cast<int>(mFirstOccupiedRow); --rowIndex)
    {
		std::size_t colIndex = 0;
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


const Grid& GameState::grid() const
{
    return mGrid;
}


void GameState::setGrid(const Grid& inGrid)
{
    Assert(mGrid.rowCount() == inGrid.rowCount() && mGrid.columnCount() == inGrid.columnCount());
    mGrid = inGrid;
    mTainted = true;
    updateCache();
}


bool GameState::tainted() const
{
    return mTainted;
}


void GameState::updateCache()
{
    mFirstOccupiedRow = 0;
    while (mFirstOccupiedRow < mGrid.rowCount())
    {
        for (std::size_t colIndex = 0; colIndex != mGrid.columnCount(); ++colIndex)
        {
            if (mGrid.get(mFirstOccupiedRow, colIndex) != BlockType_Nil)
            {
                return;
            }
        }
        mFirstOccupiedRow++;
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

const Block& GameState::originalBlock() const
{
    return mOriginalBlock;
}


std::unique_ptr<GameState> GameState::commit(const Block& inBlock, GameOver inGameOver) const
{
    std::unique_ptr<GameState> result(new GameState(*this));
    result->mIsGameOver = inGameOver.get();
    result->mTainted = false; // a new generation, a new start
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


const GameState& EvaluatedGameState::gameState() const
{
    return *mGameState;
}


GameState& EvaluatedGameState::gameState()
{
    return *mGameState;
}


int EvaluatedGameState::quality() const
{
    return mQuality;
}


} // namespace Tetris

