#include "GameState.h"
#include <assert.h>


namespace Tetris
{


    GameState::GameState(int inNumRows, int inNumColumns) :
        mGrid(inNumRows, inNumColumns, BlockType_Unknown),
        mNumLines(0),
        mNumSingles(0),
        mNumDoubles(0),
        mNumTriples(0),
        mNumTetrises(0),
        mDirty(true),
        mCachedScore(0),
        mNumHoles(0)
    {
    }


    int GameState::numLines() const
    {
        return mNumLines;
    }


    int GameState::numDoubles() const
    {
        return mNumDoubles;
    }


    int GameState::numTriples() const
    {
        
        return mNumTriples;
    }


    int GameState::numTetrises() const
    {
        return mNumTetrises;
    }


    const Grid & GameState::grid() const
    {
        return mGrid;
    }


    Grid & GameState::grid()
    {
        return mGrid;
    }


    int GameState::quality() const
    {
        if (mDirty)
        {
            static const int cHolePenalty = 2;
            static const int cHeightPenalty = 1;
            int result = 0;
            mNumHoles = 0;

            bool foundTop = false;
            size_t top = mGrid.numRows();

            size_t numOccupiedUnderTop = 0;

            for (size_t rowIdx = 0; rowIdx != mGrid.numRows(); ++rowIdx)
            {
                for (size_t colIdx = 0; colIdx != mGrid.numColumns(); ++colIdx)
                {
                    const int & value = mGrid.get(rowIdx, colIdx);
                    if (value != BlockType_Unknown)
                    {
                        if (!foundTop)
                        {
                            top = rowIdx;
                            foundTop = true;
                        }

                        if (foundTop)
                        {
                            numOccupiedUnderTop++;
                        }
                    }
                    else
                    {
                        // check for holes
                        if (foundTop && rowIdx > 0)
                        {
                            if (mGrid.get(rowIdx - 1, colIdx) != BlockType_Unknown)
                            {
                                mNumHoles++;
                                result -= cHolePenalty;
                            }
                        }
                    }
                }
            }

            result -= static_cast<int>(mGrid.numRows() - top) * cHeightPenalty;
            float density = ((float)numOccupiedUnderTop) / (float)(((mGrid.numRows() - top) * mGrid.numColumns()));
            result += static_cast<int>((11.0*density) + 0.5);
            mCachedScore = result;
            mDirty = false;
        }
        return mCachedScore;
    }


    int GameState::numHoles() const
    {
        return mNumHoles;
    }


    bool GameState::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
    {
        const size_t maxRows = mGrid.numRows() - inBlock.grid().numRows();
        const size_t maxCols = mGrid.numColumns() - inBlock.grid().numColumns();
        for (size_t r = 0; r != inBlock.grid().numRows(); ++r)
        {
            for (size_t c = 0; c != inBlock.grid().numColumns(); ++c)
            {
                if (inBlock.grid().get(r, c) != 0 && mGrid.get(inRowIdx + r, inColIdx + c) != BlockType_Unknown)
                {
                    return false;
                }
            }
        }
        return true;
    }


    GameState GameState::makeGameStateWithAddedBlock(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
    {
        GameState result(*this);
        result.mDirty = true;
        for (size_t r = 0; r != inBlock.grid().numRows(); ++r)
        {
            for (size_t c = 0; c != inBlock.grid().numColumns(); ++c)
            {
                if (inBlock.grid().get(r, c) != BlockType_Unknown)
                {
                    result.grid().set(inRowIdx + r, inColIdx + c, inBlock.type());
                }
            }
        }
        return result;
    }

} // namespace Tetris
