#include "Tetris/GameState.h"
#include <algorithm>


namespace Tetris
{

    GameState::GameState(size_t inNumRows, size_t inNumColumns) :
        mGrid(inNumRows, inNumColumns, BlockType_Nil),
        mStats(),
        mIsGameOver(false),
        mOriginalBlock(BlockType_L, Rotation(0), Row(0), Column(0)),
        mQuality()
    {
        mStats.mFirstOccupiedRow = inNumRows;
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


    int GameState::quality() const
    {
        Assert(mQuality.isInitialized());
        return mQuality.score();
    }


    bool GameState::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
    {
        if (inRowIdx >= mGrid.numRows() || inColIdx >= mGrid.numColumns())
        {
            return false;
        }

        for (size_t r = 0; r != inBlock.grid().numRows(); ++r)
        {
            for (size_t c = 0; c != inBlock.grid().numColumns(); ++c)
            {

                if (inBlock.grid().get(r, c) != 0)
                {
                    size_t rowIdx = inRowIdx + r;
                    if (rowIdx >= mGrid.numRows())
                    {
                        return false;
                    }

                    size_t colIdx = inColIdx + c;
                    if (colIdx >= mGrid.numColumns())
                    {
                        return false;
                    }

                    if (mGrid.get(rowIdx, colIdx) != BlockType_Nil)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
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


    const GameState::Stats & GameState::stats() const
    {
        return mStats;
    }


    int GameState::Stats::score() const
    {
        return 40 * mNumSingles + 100 * mNumDoubles + 300 * mNumTriples + 1200 * mNumTetrises;
    }


    void GameState::solidifyBlock(const Block & inBlock)
    {
        const Grid & grid = inBlock.grid();
        for (size_t r = 0; r != grid.numRows(); ++r)
        {
            for (size_t c = 0; c != grid.numColumns(); ++c)
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
        size_t numLines = 0;
        std::vector<bool> lines(mGrid.numRows(), false);

        // Get numLines and lines
        {
            for (size_t rowIndex = mOriginalBlock.row();
                    rowIndex < std::min<size_t>(mGrid.numRows(),
                                                mOriginalBlock.row() + mOriginalBlock.grid().numRows());
                    ++rowIndex)
            {
                lines[rowIndex] = true;
                for (size_t ci = 0; ci != mGrid.numColumns(); ++ci)
                {
                    if (mGrid.get(rowIndex, ci) == 0)
                    {
                        lines[rowIndex] = false;
                        break;
                    }
                }

                if (lines[rowIndex])
                {
                    numLines++;
                }
            }
        }

        if (numLines == 0)
        {
            return;
        }

        Assert(mStats.mFirstOccupiedRow + numLines <= mGrid.numRows());
        mStats.mFirstOccupiedRow += numLines;

        // Get newGrid
        Grid newGrid(mGrid.numRows(), mGrid.numColumns(), BlockType_Nil);
        int newRowIdx = numLines;
        for (size_t r = 0; r != mGrid.numRows(); ++r)
        {
            if (!lines[r])
            {
                for (size_t c = 0; c != mGrid.numColumns(); ++c)
                {
                    newGrid.set(newRowIdx, c, mGrid.get(r, c));
                }
                newRowIdx++;
            }
        }

        mGrid = newGrid;

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


} // namespace Tetris

